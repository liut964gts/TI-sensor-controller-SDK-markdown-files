//*****************************************************************************
//  SENSOR CONTROLLER STUDIO EXAMPLE: UART EMULATOR FOR LAUNCHPAD
//  Operating system: None
//
//  The Sensor Controller runs a UART Emulator, which implements an full-duplex
//  UART interface using bit-banging. The application example uses a small
//  subset of the supplied driver API to implement loopback, echoing each
//  received character (57600 baud, 8-N-2).
//
//  Use a terminal window to connect to the LaunchPad's XDS110 Application/User
//  USB serial port.
//
//  This example application does not implement power management.
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


// Display error message if the SCIF driver has been generated with incorrect operating system setting
#ifndef SCIF_OSAL_NONE_H
    #error "SCIF driver has incorrect operating system configuration for this example. Please change to 'None' in the Sensor Controller Studio project panel and re-generate the driver."
#endif

// Display error message if the SCIF driver has been generated with incorrect target chip package
#if !(defined(SCIF_TARGET_CHIP_PACKAGE_QFN48_7X7_RGZ) || defined(SCIF_TARGET_CHIP_PACKAGE_7X7_MOT))
    #error "SCIF driver has incorrect target chip package configuration for this example. Please change to 'QFN48 7x7 RGZ' in the Sensor Controller Studio project panel and re-generate the driver."
#endif


#define BV(n)                   (1 << (n))




void main(void) {

#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC13X2_V1) && !defined(DeviceFamily_CC13X2_V2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC26X2_V1) && !defined(DeviceFamily_CC26X2_V2) && !defined(DeviceFamily_CC26X2X7)
    // In this example, we keep the AUX domain access permanently enabled
    scifOsalEnableAuxDomainAccess();
#endif

    // Initialize and start the Sensor Controller
    scifInit(&scifDriverSetup);

    // Start the UART emulator
    scifExecuteTasksOnceNbl(BV(SCIF_UART_EMULATOR_TASK_ID));

    // Enable baud rate generation
    scifUartSetBaudRate(57600);

    // Enable RX
    scifUartSetRxTimeout(20);
    scifUartSetRxEnableReqIdleCount(1);
    scifUartRxEnable(1);

    // Enable events
    scifUartSetEventMask(0xF);

    // Main loop
    while (1) {

        // Loop back any received characters
        while (scifUartGetRxFifoCount()) {
            scifUartTxPutChar((char) scifUartRxGetChar());
        }
    }

} // main
