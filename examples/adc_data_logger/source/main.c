//*****************************************************************************
//  SENSOR CONTROLLER STUDIO EXAMPLE: ADC DATA LOGGER FOR LAUNCHPAD
//  Operating system: None
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
//  Use a terminal window to connect to the LaunchPad's USB Serial Port.
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
#include "ex_include.h"
#include "scif.h"
#include "string.h"
#include "stdio.h"


#define BV(n)               (1 << (n))


// Display error message if the SCIF driver has been generated with incorrect operating system setting
#ifndef SCIF_OSAL_NONE_H
    #error "SCIF driver has incorrect operating system configuration for this example. Please change to 'None' in the Sensor Controller Studio project panel and re-generate the driver."
#endif

// Display error message if the SCIF driver has been generated with incorrect target chip package
#if !(defined(SCIF_TARGET_CHIP_PACKAGE_QFN48_7X7_RGZ) || defined(SCIF_TARGET_CHIP_PACKAGE_7X7_MOT))
    #error "SCIF driver has incorrect target chip package configuration for this example. Please change to 'QFN48 7x7 RGZ' in the Sensor Controller Studio project panel and re-generate the driver."
#endif


// Line buffer for UART printing
char pLine[32];


// I/O pin mapping
#define BV(n)                   (1 << (n))
#define IOC_O_IOCFG(n)          (IOC_O_IOCFG0 + (sizeof(uint32_t) * n))
#define IOID_UART_RXD           IOID_UNUSED
#if defined(SCIF_TARGET_CHIP_NAME_CC1352R1F3) || defined(SCIF_TARGET_CHIP_NAME_CC1352P1F3) || defined(SCIF_TARGET_CHIP_NAME_CC1352P7) || defined(SCIF_TARGET_CHIP_NAME_CC2652P1F) || defined(SCIF_TARGET_CHIP_NAME_CC2652RSIP) || defined(SCIF_TARGET_CHIP_NAME_CC2652PSIP) || defined(SCIF_TARGET_CHIP_NAME_CC2652P7) || defined(SCIF_TARGET_CHIP_NAME_CC2672R3) || defined(SCIF_TARGET_CHIP_NAME_CC2672P3)
    #define IOID_UART_TXD       IOID_13
#else
    #define IOID_UART_TXD       IOID_3
#endif
#define IOID_UART_CTS           IOID_UNUSED
#define IOID_UART_RTS           IOID_UNUSED




void scTaskAlertCallback(void) {

} // scTaskAlertCallback




void scCtrlReadyCallback(void) {

} // scCtrlReadyCallback




static void uartTxString(char* pText) {
    while (*pText != '\0') {
        while (HWREG(UART0_BASE + UART_O_FR) & UART_FR_TXFF);
        HWREG(UART0_BASE + UART_O_DR) = *(pText++);
    }
} // uartTxString




int main(void) {

    // Enable power domains
    PRCMPowerDomainOn(PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_SERIAL);
#ifdef PRCMPowerDomainsAllOn
    while (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_SERIAL) != PRCM_DOMAIN_POWER_ON);
#else
    while (PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_SERIAL) != PRCM_DOMAIN_POWER_ON);
#endif

    // Enable peripheral clocks
    PRCMPeripheralRunEnable(PRCM_PERIPH_UART0);
    PRCMPeripheralRunEnable(PRCM_PERIPH_GPIO);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    // Map UART0 to DIO pins
    IOCPinTypeUart(UART0_BASE, IOID_UART_RXD, IOID_UART_TXD, IOID_UART_CTS, IOID_UART_RTS);

    // Configure and enable the UART (57600, 8-N-1)
    UARTConfigSetExpClk(UART0_BASE, 48000000, 57600, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    UARTEnable(UART0_BASE);

#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC13X2_V1) && !defined(DeviceFamily_CC13X2_V2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC26X2_V1) && !defined(DeviceFamily_CC26X2_V2) && !defined(DeviceFamily_CC26X2X7)
    // In this example, we keep the AUX domain access permanently enabled
    scifOsalEnableAuxDomainAccess();
#endif

    // Initialize and start the Sensor Controller
    scifOsalRegisterCtrlReadyCallback(scCtrlReadyCallback);
    scifOsalRegisterTaskAlertCallback(scTaskAlertCallback);
    scifInit(&scifDriverSetup);
    scifStartRtcTicksNow(0x00010000 / 10);
    IntMasterEnable();
    AONRTCEnable();

    // Configure and start the ADC Data Logger task. The task does not signalize data exchange, but
    // has buffering capacity for 128 samples = 12.8 seconds (which are polled every 10 seconds below)
    scifStartTasksNbl(BV(SCIF_ADC_DATA_LOGGER_TASK_ID));

    // Maintain the sample buffer tail index here. The Sensor Controller increments the head index (also
    // starting at 0) each time a sample is stored in the buffer.
    uint16_t tail = 0;

    // Main loop
    int rtcNextSecValue = 10;
    while (1) {

        // Wake up every 10 seconds
        while (AONRTCSecGet() < rtcNextSecValue);
        rtcNextSecValue += 10;

        // Fetch the current head index
        uint16_t head = scifTaskData.adcDataLogger.output.head;

        // Initialize statistics to be outputted after each chunk of ADC values
        uint16_t count = (head - tail + SCIF_ADC_DATA_LOGGER_BUFFER_SIZE) % SCIF_ADC_DATA_LOGGER_BUFFER_SIZE;
        uint32_t sum = 0;
        uint16_t min = 0xFFFF;
        uint16_t max = 0;

        // Until we've caught up with the Sensor Controller ...
        while (tail != head) {

            // Output the ADC value over UART
            uint16_t value = scifTaskData.adcDataLogger.output.pSamples[tail];
            sprintf(pLine, "%d\r\n", value);
            uartTxString(pLine);

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
        uartTxString(pLine);
        sprintf(pLine, "Average: %d\r\n", sum / count);
        uartTxString(pLine);
        sprintf(pLine, "Minimum: %d\r\n", min);
        uartTxString(pLine);
        sprintf(pLine, "Maximum: %d\r\n", max);
        uartTxString(pLine);
   }

} // main
