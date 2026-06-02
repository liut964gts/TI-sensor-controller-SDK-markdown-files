//*****************************************************************************
//  SENSOR CONTROLLER STUDIO EXAMPLE: TASK CONTROL FOR LAUNCHPAD
//  Operating system: TI-RTOS
//
//  Demonstrates use of multiple Sensor Controller tasks in one SCIF driver,
//  with interaction and output using the LaunchPad buttons, LEDs and UART.
//
//  The following Sensor Controller tasks are implemented:
//  - ADC Sampler: Samples a single ADC input 500 times at 20 kHz. When the
//    sampling is completed, all 500 samples are outputted over UART.
//      - Press the BTN2 button to execute the task once.
//      - Red LED is on while the task is running.
// - Battery Voltage Monitor: Samples VDDS using the ADC, and notifies the
//   application when below a configurable alarm threshold. The application is
//   also notified when returning above the threshold. The task runs off the
//   RTC at 1 Hz.
//      - Always enabled
// - LED Fader: Fades up the Green LED repeatedly. The task runs off the Timer
//   Event Trigger with varying interval.
//      - Press the BTN1 button to start the task
//      - Release the BTN1 button to stop the task
//
//  Use a terminal window to connect to the LaunchPad's XDS110 Application/User
//  USB serial port (57600 baud, 8-N-1).
//
//
//  Copyright (C) 2017 Texas Instruments Incorporated - https://www.ti.com/
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


#define BV(n)               (1 << (n))


#define EV_BTN1_EDGE        (Event_Id_00)
#define EV_BTN2_EDGE        (Event_Id_01)

#define EV_BTNS_MASK        ((EV_BTN1_EDGE) | (EV_BTN2_EDGE))


// Display error message if the SCIF driver has been generated with incorrect operating system setting
#if !(defined(SCIF_OSAL_TIRTOS_H) || defined(SCIF_OSAL_TIDPL_H))
    #error "SCIF driver has incorrect operating system configuration for this example. Please change to 'TI-RTOS' or 'TI Driver Porting Layer' in the Sensor Controller Studio project panel and re-generate the driver."
#endif

// Display error message if the SCIF driver has been generated with incorrect target chip package
#if !(defined(SCIF_TARGET_CHIP_PACKAGE_QFN48_7X7_RGZ) || defined(SCIF_TARGET_CHIP_PACKAGE_7X7_MOT))
    #error "SCIF driver has incorrect target chip package configuration for this example. Please change to 'QFN48 7x7 RGZ' in the Sensor Controller Studio project panel and re-generate the driver."
#endif


// Task data
Task_Struct myOutputTask;
Char myOutputTaskStack[1024];
Task_Struct myButtonTask;
Char myButtonTaskStack[1024];


// Semaphore used to wait for Sensor Controller task ALERT event
static Semaphore_Struct semScTaskAlert;

// Semaphore used to wait for ADC sampler ALERT processing to complete
static Semaphore_Struct semAdcSamplerProcDone;

// Event used to wait for button edges
static Event_Struct evnButtonEdge;
static Event_Handle hEvnButtonEdge;


// UART driver objects
UART_Handle uHandle;
UART_Params uParams;

// Line buffer for UART printing
char pLine[128];




void scCtrlReadyCallback(void) {

} // scCtrlReadyCallback




void scTaskAlertCallback(void) {

    // Wake up the OS task
    Semaphore_post(Semaphore_handle(&semScTaskAlert));

} // scTaskAlertCallback




void buttonEdgeCallback(uint_least8_t index) {

    // Disable button pin interrupts
    GPIO_setInterruptConfig(Board_BTN1, GPIO_CFG_IN_INT_NONE | GPIO_CFG_INT_DISABLE);
    GPIO_setInterruptConfig(Board_BTN2, GPIO_CFG_IN_INT_NONE | GPIO_CFG_INT_DISABLE);

    // Resume the button OS task
    if (index == Board_BTN1) {
        Event_post(hEvnButtonEdge, EV_BTN1_EDGE);
    } else if (index == Board_BTN2) {
        Event_post(hEvnButtonEdge, EV_BTN2_EDGE);
    }

} // buttonEdgeCallback




uint16_t millivoltsToAdcValue(int32_t millivolts) {
    int32_t adcValue = AUXADCMicrovoltsToValue(AUXADC_FIXED_REF_VOLTAGE_NORMAL, millivolts * 1000);
    return AUXADCUnadjustValueForGainAndOffset(adcValue,
                                               AUXADCGetAdjustmentGain(AUXADC_REF_FIXED),
                                               AUXADCGetAdjustmentOffset(AUXADC_REF_FIXED));
} // millivoltsToAdcValue




uint16_t adcValueToMillivolts(int32_t adcValue) {
    adcValue = AUXADCAdjustValueForGainAndOffset(adcValue,
                                                 AUXADCGetAdjustmentGain(AUXADC_REF_FIXED),
                                                 AUXADCGetAdjustmentOffset(AUXADC_REF_FIXED));
    return AUXADCValueToMicrovolts(AUXADC_FIXED_REF_VOLTAGE_NORMAL, adcValue) / 1000;
} // millivoltsToAdcValue




void outputTaskFxn(UArg a0, UArg a1) {

    // Initialize UART parameters
    UART_Params_init(&uParams);
    uParams.baudRate      = 57600;
    uParams.writeDataMode = UART_DATA_TEXT;
    uParams.dataLength    = UART_LEN_8;
    uParams.stopBits      = UART_STOP_ONE;

    // Main loop
    while (1) {
        // Wait for an ALERT callback
        Semaphore_pend(Semaphore_handle(&semScTaskAlert), BIOS_WAIT_FOREVER);

        // Clear the ALERT interrupt source
        scifClearAlertIntSource();

        // Get the alert events
        uint32_t bvAlertEvents = scifGetAlertEvents();

        // Open UART
        uHandle = UART_open(Board_UART, &uParams);

        // If the ADC sampler task has finished ...
        if (bvAlertEvents & BV(SCIF_ADC_SAMPLER_TASK_ID)) {

            // Output all samples over UART, one per line
            for (int n = 0; n < SCIF_ADC_SAMPLER_BUFFER_SIZE; n++) {
                sprintf(pLine, "ADC sample %d: %d\r\n", n, scifTaskData.adcSampler.output.pSamples[n]);
                UART_write(uHandle, pLine, strlen(pLine));
            }

            // Indicate processing completion
            Semaphore_post(Semaphore_handle(&semAdcSamplerProcDone));
        }

        // If the battery voltage monitor has generated an alarm ...
        if (bvAlertEvents & BV(SCIF_BATTERY_VOLTAGE_MONITOR_TASK_ID)) {

            // Indicate alarm status on Red LED
            uint16_t alarm = scifTaskData.batteryVoltageMonitor.output.alarm;
            GPIO_write(Board_RLED, alarm);

            // Convert ADC value to voltage
            int batteryVoltage = adcValueToMillivolts(scifTaskData.batteryVoltageMonitor.output.adcValue);

            // Output alarm status and battery voltage over UART
            if (alarm) {
                sprintf(pLine, "Battery voltage OK: %d mV\r\n", batteryVoltage);
            } else {
                sprintf(pLine, "Battery voltage ALARM: %d mV\r\n", batteryVoltage);
            }
            UART_write(uHandle, pLine, strlen(pLine));
        }

        // Close UART
        UART_close(uHandle);

        // Acknowledge the ALERT event
        scifAckAlertEvents();
    }

} // outputTaskFxn




void buttonTaskFxn(UArg a0, UArg a1) {

    // Enable the LED pin
    GPIO_setConfig(Board_GLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    // Enable button pins, with interrupt handler buttonEdgeCallback()
    GPIO_setCallback(Board_BTN1, buttonEdgeCallback);
    GPIO_setCallback(Board_BTN2, buttonEdgeCallback);
    GPIO_setConfig(Board_BTN1, GPIO_CFG_IN_PU | GPIO_CFG_HYSTERESIS_ON);
    GPIO_setConfig(Board_BTN2, GPIO_CFG_IN_PU | GPIO_CFG_HYSTERESIS_ON);

    // Initialize the Sensor Controller
    scifOsalInit();
    scifOsalRegisterCtrlReadyCallback(scCtrlReadyCallback);
    scifOsalRegisterTaskAlertCallback(scTaskAlertCallback);
    scifInit(&scifDriverSetup);
    scifStartRtcTicksNow(0x00010000 / 1); // 1 Hz for battery voltage monitor

    // Run the Battery Voltage Monitor task indefinitely at 1 Hz (off the RTC)
    scifTaskData.batteryVoltageMonitor.cfg.alarmEnable = 1;
    scifTaskData.batteryVoltageMonitor.cfg.alarmThr = millivoltsToAdcValue(2700);
    scifStartTasksNbl(BV(SCIF_BATTERY_VOLTAGE_MONITOR_TASK_ID));

    // Configure the LED Fader task
    scifTaskData.ledFader.cfg.pwmRepeatCount = 10;
    scifTaskData.ledFader.cfg.pwmPeriod      = 10;

    // Main loop
    while (1) {

        // Wait for the user to press a button:
        // - BTN1: Run the LED Fader task until the button is released
        // - BTN2: Run the ADC Sampling task once

        // Setup interrupt on both buttons
        GPIO_clearInt(Board_BTN1);
        GPIO_clearInt(Board_BTN2);
        GPIO_setInterruptConfig(Board_BTN1, GPIO_CFG_IN_INT_FALLING | GPIO_CFG_INT_ENABLE);
        GPIO_setInterruptConfig(Board_BTN2, GPIO_CFG_IN_INT_FALLING | GPIO_CFG_INT_ENABLE);

        // Wait for the user to press BTN1 or BTN2
        UInt bvPressEvents = Event_pend(hEvnButtonEdge, Event_Id_NONE, EV_BTNS_MASK, BIOS_WAIT_FOREVER);

        // If BTN1 was pressed ...
        if (bvPressEvents & EV_BTN1_EDGE) {

            // Start the LED Fader task
            while (scifWaitOnNbl(0) != SCIF_SUCCESS);
            scifResetTaskStructs(BV(SCIF_LED_FADER_TASK_ID), 0);
            scifStartTasksNbl(BV(SCIF_LED_FADER_TASK_ID));

            // Debounce the button press by waiting for 200 ms
            Task_sleep(200000 / Clock_tickPeriod);

            // Setup interrupt on BTN1 and wait for the user to release it
            GPIO_clearInt(Board_BTN1);
            GPIO_setInterruptConfig(Board_BTN1, GPIO_CFG_IN_INT_RISING | GPIO_CFG_INT_ENABLE);

            // Handle the case where BTN1 already has been released
            if (GPIO_read(Board_BTN1) == 1) {
                GPIO_setInterruptConfig(Board_BTN1, GPIO_CFG_IN_INT_NONE | GPIO_CFG_INT_DISABLE);
                Event_post(hEvnButtonEdge, EV_BTN1_EDGE);
            }

            // Wait for the user to release BTN1
            UInt bvPressEvents = Event_pend(hEvnButtonEdge, Event_Id_NONE, EV_BTNS_MASK, BIOS_WAIT_FOREVER);

            // Stop the LED Fader task
            while (scifWaitOnNbl(0) != SCIF_SUCCESS);
            scifStopTasksNbl(BV(SCIF_LED_FADER_TASK_ID));

            // Debounce the button release by waiting for 200 ms
            Task_sleep(200000 / Clock_tickPeriod);

        // Otherwise, if BTN2 was pressed ...
        } else if (bvPressEvents & EV_BTN2_EDGE) {

            // Turn on Red LED to indicate measurement in progress
            GPIO_write(Board_RLED, 1);

            // Execute the ADC Sampler task
            while (scifWaitOnNbl(0) != SCIF_SUCCESS);
            scifResetTaskStructs(BV(SCIF_ADC_SAMPLER_TASK_ID), BV(SCIF_STRUCT_OUTPUT));
            scifExecuteTasksOnceNbl(BV(SCIF_ADC_SAMPLER_TASK_ID));

            // This will take some time to complete, so wait for it using a semaphore
            Semaphore_pend(Semaphore_handle(&semAdcSamplerProcDone), BIOS_WAIT_FOREVER);

            // Turn off Red LED to indicate measurement and processing completed
            GPIO_write(Board_RLED, 0);

            // Not debouncing this button, since it is almost two seconds since it was pressed
        }
    }

} // buttonTaskFxn




int main(void) {
    Task_Params taskParams;
    Semaphore_Params semParams;
    Event_Params evnParams;

    // Initialize the board
    Board_initGeneral();
#ifdef Board_shutDownExtFlash
    Board_shutDownExtFlash();
#endif

    // Initialize the UART driver
    UART_init();

    // Configure the output OS task (handles Sensor Controller ALERT interrupts)
    Task_Params_init(&taskParams);
    taskParams.stack = myOutputTaskStack;
    taskParams.stackSize = sizeof(myOutputTaskStack);
    taskParams.priority = 3;
    Task_construct(&myOutputTask, outputTaskFxn, &taskParams, NULL);

    // Configure the button OS task (handles Sensor Controller task control)
    Task_Params_init(&taskParams);
    taskParams.stack = myButtonTaskStack;
    taskParams.stackSize = sizeof(myButtonTaskStack);
    taskParams.priority = 3;
    Task_construct(&myButtonTask, buttonTaskFxn, &taskParams, NULL);

    // Create the semaphore used to wait for Sensor Controller ALERT events
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&semScTaskAlert, 0, &semParams);

    // Create the semaphore used to wait for ADC sampler ALERT processing to complete
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&semAdcSamplerProcDone, 0, &semParams);

    // Create the event used to wait for button edges
    Event_Params_init(&evnParams);
    Event_construct(&evnButtonEdge, &evnParams);
    hEvnButtonEdge = (Event_Handle)&evnButtonEdge;

    // Start TI-RTOS
    BIOS_start();
    return 0;

} // main
