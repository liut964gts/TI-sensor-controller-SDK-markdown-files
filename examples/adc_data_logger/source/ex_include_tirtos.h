#ifndef EX_INCLUDE_TIRTOS_H
#define EX_INCLUDE_TIRTOS_H


#include <ti/devices/DeviceFamily.h>
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/gpio/GPIOCC26XX.h>
#include <ti/drivers/UART2.h>
#include DeviceFamily_constructPath(driverlib/aux_adc.h)
#include "ti_drivers_config.h"


// Handle renaming between SDK releases
#ifndef Board_LED_ON
    #define Board_LED_ON    CONFIG_GPIO_LED_ON
#endif
#ifndef Board_LED_OFF
    #define Board_LED_OFF   CONFIG_GPIO_LED_OFF
#endif
#ifndef Board_RLED
    #define Board_RLED      CONFIG_GPIO_LED_0
#endif
#ifndef Board_GLED
    #define Board_GLED      CONFIG_GPIO_LED_1
#endif
#ifndef Board_BTN1
    #define Board_BTN1      CONFIG_GPIO_BTN_0
#endif
#ifndef Board_BTN2
    #define Board_BTN2      CONFIG_GPIO_BTN_1
#endif
#ifndef Board_UART
    #define Board_UART      CONFIG_UART_0
#endif


#endif
