//*****************************************************************************
//  SENSOR CONTROLLER STUDIO EXAMPLE: ADC DATA LOGGER FOR LAUNCHPAD
//  Operating system: TI-RTOS
//
//  The Sensor Controller is used to sample and buffer a single ADC channel.
//  The ADC samples are stored in a ring-buffer, and the Sensor Controller
//  maintains a head index indicating where the next sample will be written.
//  The sampling interval is specified in the call to scifStartRtcTicksNow().
//
//  The application wakes up at a fixed interval that is asynchronous to the
//  Sensor Controller wake-ups, and transfers over UART (57600 baud, 8-N-1):
//  - The ADC samples
//  - Statistics: Number of samples, average, minimum and maximum
//
//  Use a terminal window to connect to the LaunchPad's XDS110 Application/User
//  USB serial port.
//
//
//  Copyright (C) 2014 Texas Instruments Incorporated - https://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/
#include "ex_include_tirtos.h"
#include "scif.h"
#include "string.h"
#include "stdio.h"

#include "ti/drivers/UART2.h"


#define BV(n)               (1 << (n))


// Display error message if the SCIF driver has been generated with incorrect operating system setting
#if !(defined(SCIF_OSAL_TIRTOS_H) || defined(SCIF_OSAL_TIDPL_H))
    #error "SCIF driver has incorrect operating system configuration for this example. Please change to 'TI-RTOS' or 'TI Driver Porting Layer' in the Sensor Controller Studio project panel and re-generate the driver."
#endif

// Display error message if the SCIF driver has been generated with incorrect target chip package
#if !(defined(SCIF_TARGET_CHIP_PACKAGE_QFN48_7X7_RGZ) || defined(SCIF_TARGET_CHIP_PACKAGE_7X7_MOT))
    #error "SCIF driver has incorrect target chip package configuration for this example. Please change to 'QFN48 7x7 RGZ' in the Sensor Controller Studio project panel and re-generate the driver."
#endif


// Task data
Task_Struct myTask;
Char myTaskStack[1024];


// UART driver objects
UART2_Handle uHandle;
UART2_Params uParams;

// Line buffer for UART printing
char pLine[32];




void scTaskAlertCallback(void) {

} // taskAlertCallback




void scCtrlReadyCallback(void) {

} // ctrlReadyCallback




void taskFxn(UArg a0, UArg a1) {

    // Initialize UART parameters
    UART2_Params_init(&uParams);
    uParams.baudRate      = 57600;
    // uParams.writeMode = UART_DATA_TEXT;
    // uParams.dataLength    = UART_LEN_8;
    // uParams.stopBits      = UART_STOP_ONE;

    // Initialize the Sensor Controller
    scifOsalInit();
    scifOsalRegisterCtrlReadyCallback(scCtrlReadyCallback);
    scifOsalRegisterTaskAlertCallback(scTaskAlertCallback);
    scifInit(&scifDriverSetup);
    scifStartRtcTicksNow(0x00010000 / 10);

    // Configure and start the ADC Data Logger task. The task does not signalize data exchange, but
    // has buffering capacity for 128 samples = 12.8 seconds (which are polled every 10 seconds below)
    scifStartTasksNbl(BV(SCIF_ADC_DATA_LOGGER_TASK_ID));

    // Maintain the sample buffer tail index here. The Sensor Controller increments the head index (also
    // starting at 0) each time a sample is stored in the buffer.
    uint16_t tail = 0;

    // Main loop
    while (1) {

        // Wake up every 10 seconds
        Task_sleep(10000000 / Clock_tickPeriod);

        // Open UART
        uHandle = UART2_open(Board_UART, &uParams);

        // Fetch the current head index
        uint16_t head = scifTaskData.adcDataLogger.output.head;

        // Initialize statistics to be outputted after each chunk of ADC values
        uint16_t count = (head - tail + SCIF_ADC_DATA_LOGGER_BUFFER_SIZE) % SCIF_ADC_DATA_LOGGER_BUFFER_SIZE;
        uint32_t sum = 0;
        uint16_t min = 0xFFFF;
        uint16_t max = 0;

        size_t byteWritten = 0;

        // Until we've caught up with the Sensor Controller ...
        while (tail != head) {

            // Output the ADC value over UART
            
            uint16_t value = scifTaskData.adcDataLogger.output.pSamples[tail];
            sprintf(pLine, "%d\r\n", value);
            UART2_write(uHandle, pLine, strlen(pLine),  &byteWritten);

            // Update statistics
            sum += value;
            if (value < min) min = value;
            if (value > max) max = value;

            // Increment the tail index
            if (++tail >= SCIF_ADC_DATA_LOGGER_BUFFER_SIZE) {
                tail = 0;
            }
        }

        // Output statistics over UART
        sprintf(pLine, "Count:   %d\r\n", count);
        UART2_write(uHandle, pLine, strlen(pLine),&byteWritten);
        sprintf(pLine, "Average: %d\r\n", sum / count);
        UART2_write(uHandle, pLine, strlen(pLine),&byteWritten);
        sprintf(pLine, "Minimum: %d\r\n", min);
        UART2_write(uHandle, pLine, strlen(pLine),&byteWritten);
        sprintf(pLine, "Maximum: %d\r\n", max);
        UART2_write(uHandle, pLine, strlen(pLine),&byteWritten);

        // Close UART
        UART2_close(uHandle);
   }

} // taskFxn




int main(void) {
    Task_Params taskParams;

    // Initialize the board
    Board_initGeneral();
#ifdef Board_shutDownExtFlash
    Board_shutDownExtFlash();
#endif

    // Initialize the UART driver
    // UART2_init();

    // Configure the OS task
    Task_Params_init(&taskParams);
    taskParams.stack = myTaskStack;
    taskParams.stackSize = sizeof(myTaskStack);
    taskParams.priority = 3;
    Task_construct(&myTask, taskFxn, &taskParams, NULL);

    // Start TI-RTOS
    BIOS_start();
    return 0;

} // main
