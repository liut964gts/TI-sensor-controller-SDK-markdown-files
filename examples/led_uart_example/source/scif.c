/// \addtogroup module_scif_driver_setup
//@{
#include "scif.h"
#include "scif_framework.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_aon_event.h)
#include DeviceFamily_constructPath(inc/hw_aon_rtc.h)
#include DeviceFamily_constructPath(inc/hw_aon_pmctl.h)
#include DeviceFamily_constructPath(inc/hw_aux_sce.h)
#include DeviceFamily_constructPath(inc/hw_aux_smph.h)
#include DeviceFamily_constructPath(inc/hw_aux_spim.h)
#include DeviceFamily_constructPath(inc/hw_aux_evctl.h)
#include DeviceFamily_constructPath(inc/hw_aux_aiodio.h)
#include DeviceFamily_constructPath(inc/hw_aux_timer01.h)
#include DeviceFamily_constructPath(inc/hw_aux_sysif.h)
#include DeviceFamily_constructPath(inc/hw_event.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_ioc.h)
#include <string.h>
#if defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h>
#endif


// OSAL function prototypes
uint32_t scifOsalEnterCriticalSection(void);
void scifOsalLeaveCriticalSection(uint32_t key);




/// Firmware image to be uploaded to the AUX RAM
static const uint16_t pAuxRamImage[] = {
    /*0x0000*/ 0x140E, 0x0417, 0x140E, 0x0438, 0x140E, 0x0442, 0x140E, 0x045F, 0x140E, 0x0468, 0x140E, 0x0471, 0x140E, 0x047A, 0x8953, 0x9954, 
    /*0x0020*/ 0x8D29, 0xBEFD, 0x4553, 0x2554, 0xAEFE, 0x445C, 0xADB7, 0x745B, 0x545B, 0x7000, 0x7CA6, 0x68AF, 0x00A0, 0x1431, 0x68B0, 0x00A2, 
    /*0x0040*/ 0x1431, 0x68B1, 0x00A4, 0x1431, 0x78A6, 0xF801, 0xFA02, 0xBEF2, 0x78AD, 0x68AF, 0xFD0E, 0x68B1, 0xED92, 0xFD06, 0x7CAD, 0x6440, 
    /*0x0060*/ 0x047F, 0x78A6, 0x8F1F, 0xED8F, 0xEC01, 0xBE01, 0xADB7, 0x8DB7, 0x755B, 0x555B, 0x78AB, 0x60BF, 0xEF27, 0xE240, 0xEF27, 0x7000, 
    /*0x0080*/ 0x7CAB, 0x047F, 0x6477, 0x0000, 0x18AD, 0x9D88, 0x9C01, 0xB60E, 0x109E, 0xAF19, 0xAA00, 0xB60A, 0xA8FF, 0xAF39, 0xBE07, 0x0CA6, 
    /*0x00A0*/ 0x8600, 0x88A2, 0x8F08, 0xFD47, 0x9DB7, 0x08A6, 0x8801, 0x8A02, 0xBEEB, 0x254F, 0xAEFE, 0x645B, 0x445B, 0x4477, 0x047F, 0x5656, 
    /*0x00C0*/ 0x655B, 0x455B, 0x0000, 0x0CA6, 0x0001, 0x0CA7, 0x1416, 0x047F, 0x5657, 0x665B, 0x465B, 0x0000, 0x0CA6, 0x0002, 0x0CA7, 0x1416, 
    /*0x00E0*/ 0x047F, 0x5658, 0x675B, 0x475B, 0x0000, 0x0CA6, 0x0004, 0x0CA7, 0x1416, 0x047F, 0x765B, 0x565B, 0x86FF, 0x03FF, 0x0CA9, 0x645C, 
    /*0x0100*/ 0x78A8, 0x68A9, 0xED37, 0xB605, 0x0000, 0x0CA8, 0x7CAE, 0x6540, 0x0CA9, 0x78A9, 0x68AA, 0xFD0E, 0xF801, 0xE95A, 0xFD0E, 0xBE01, 
    /*0x0120*/ 0x6553, 0xBDB7, 0x700B, 0xFB96, 0x4453, 0x2454, 0xAEFE, 0xADB7, 0x6453, 0x2454, 0xA6FE, 0x7000, 0xFB96, 0xADB7, 0x0000, 0x0000, 
    /*0x0140*/ 0x0143, 0x015E, 0x0147, 0x015F, 0x015C, 0x0161, 0x0000, 0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x0160*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0016, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x0180*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x01A0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x01C0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x01E0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x0200*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x0220*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x0240*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x0260*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    /*0x0280*/ 0x0000, 0x0000, 0x0000, 0x54BB, 0x0010, 0x0C9E, 0xADB7, 0x08B3, 0x8C01, 0xB602, 0x74BB, 0x054D, 0x54BB, 0x08B3, 0x8801, 0x0CB3, 
    /*0x02A0*/ 0x08B3, 0x8C0F, 0xBE06, 0x08B3, 0x8DAC, 0x0CB2, 0x08A8, 0x8201, 0x0CA8, 0x0001, 0x0C9E, 0xADB7, 0x54BB, 0xADB7, 0xADB7, 0x1562, 
    /*0x02C0*/ 0xADB7, 0xADB7, 0x5000, 0x28B8, 0x017D, 0x0E80, 0x01A7, 0x0E81, 0x8602, 0x0218, 0x0E82, 0x8602, 0x0269, 0x0E83, 0x8602, 0x0274, 
    /*0x02E0*/ 0x0E84, 0x0A81, 0xCDB1, 0x9DB7, 0x0A83, 0xCDB1, 0x9DB7, 0x0A81, 0xCDB1, 0x9DB7, 0x0A84, 0xCDB1, 0x8DB7, 0x6941, 0x1942, 0x9D2E, 
    /*0x0300*/ 0xB605, 0x8600, 0x98B9, 0xEF09, 0x1187, 0x1E80, 0xADB7, 0x8600, 0xEAFF, 0x9E03, 0x86FF, 0xE800, 0x8E04, 0x57BB, 0x7008, 0x1192, 
    /*0x0320*/ 0x1E80, 0xADB7, 0xEC01, 0xBE02, 0x57BB, 0x8E01, 0x77BB, 0xEDA9, 0xF8FF, 0xBE02, 0x119D, 0x1E80, 0xADB7, 0x117D, 0x1E80, 0x77BB, 
    /*0x0340*/ 0x1942, 0x9801, 0x9A40, 0xAE01, 0x1000, 0x1D42, 0xADB7, 0xDA00, 0x9E09, 0xD8FF, 0xBE07, 0x8608, 0xC200, 0xCF2B, 0x1939, 0x9202, 
    /*0x0360*/ 0x1D39, 0xADB7, 0x093D, 0x193E, 0x8D29, 0x9E0E, 0xAA00, 0xB605, 0xA8FF, 0x24C0, 0xAE01, 0x28B8, 0xADB7, 0x8602, 0x126C, 0x1E83, 
    /*0x0380*/ 0x8602, 0x1277, 0x1E84, 0x0D3E, 0xADB7, 0x8602, 0x0269, 0x0E83, 0x8602, 0x0274, 0x0E84, 0x01A7, 0x0E81, 0x5000, 0x5D3E, 0x28B8, 
    /*0x03A0*/ 0xADB7, 0x8602, 0x1269, 0x1E83, 0x8602, 0x1274, 0x1E84, 0x5008, 0x4000, 0x393F, 0x8600, 0xB8F9, 0x11DF, 0x1E81, 0xADB7, 0xCDA9, 
    /*0x03C0*/ 0x11E3, 0x1E81, 0xADB7, 0xFD47, 0x24C0, 0xA601, 0xC280, 0xD8FF, 0xBE02, 0x11EE, 0x8E01, 0x11DF, 0x1E81, 0xADB7, 0x593F, 0xD801, 
    /*0x03E0*/ 0xDA40, 0xAE01, 0x5000, 0x01F6, 0x0E81, 0xADB7, 0xFD47, 0x24C0, 0xAE0B, 0xCCFF, 0xBE03, 0x8602, 0xC200, 0x8E02, 0x8601, 0xC200, 
    /*0x0400*/ 0x1939, 0x9204, 0x1D39, 0xADB7, 0x01A7, 0x0E81, 0x8602, 0x026C, 0x0E83, 0x8602, 0x0277, 0x0E84, 0x0940, 0xDD28, 0xBE05, 0x8604, 
    /*0x0420*/ 0xC200, 0xCF2B, 0x58B7, 0xADB7, 0xCF2B, 0x5D3F, 0x58B7, 0xADB7, 0x193C, 0x9A00, 0xB601, 0xADB7, 0x093D, 0x193E, 0x8D29, 0xA602, 
    /*0x0440*/ 0x01C5, 0x0E81, 0x8602, 0x0226, 0x0E82, 0x0571, 0x08B4, 0x0D3B, 0x1939, 0x9D00, 0x9006, 0x1D39, 0x8602, 0x0230, 0x0E82, 0x0571, 
    /*0x0460*/ 0x093F, 0x1940, 0x8D19, 0xD601, 0x8840, 0x0E7F, 0x8602, 0x023A, 0x0E82, 0x0571, 0x0A7F, 0x18B5, 0x8D29, 0xAE03, 0x0939, 0x8201, 
    /*0x0480*/ 0x0D39, 0x8602, 0x0245, 0x0E82, 0x0571, 0x0941, 0x1942, 0x8D19, 0xD601, 0x8840, 0x0E7F, 0x8602, 0x024F, 0x0E82, 0x0571, 0x0A7F, 
    /*0x04A0*/ 0x18B6, 0x8D29, 0x9603, 0x0939, 0x8208, 0x0D39, 0x8602, 0x025A, 0x0E82, 0x0571, 0x093A, 0x8A00, 0xBE08, 0x0939, 0x193B, 0x8D01, 
    /*0x04C0*/ 0xB604, 0x0D3A, 0x6540, 0x0000, 0x0D39, 0x8602, 0x0218, 0x0E82, 0x0571, 0x0A80, 0xFD47, 0x8E03, 0x0A80, 0x24C0, 0xA602, 0xFD47, 
    /*0x04E0*/ 0x8E02, 0x11D1, 0x1E81, 0x8DB7, 0x0A82, 0xFD47, 0x8E03, 0x0A82, 0x24C0, 0xA602, 0xFD47, 0x8E02, 0x11D1, 0x1E81, 0x8DB7, 0x0000, 
    /*0x0500*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};


/// Look-up table that converts from AUX I/O index to MCU IOCFG offset
static const uint8_t pAuxIoIndexToMcuIocfgOffsetLut[] = {
    0, 68, 64, 60, 56, 52, 48, 44, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4, 0, 120, 116, 112, 108, 104, 100, 96, 92, 88, 84, 80, 76, 72
};


/** \brief Look-up table of data structure information for each task
  *
  * There is one entry per data structure (\c cfg, \c input, \c output and \c state) per task:
  * - [31:20] Data structure size (number of 16-bit words)
  * - [19:12] Buffer count (when 2+, first data structure is preceded by buffering control variables)
  * - [11:0] Address of the first data structure
  */
static const uint32_t pScifTaskDataStructInfoLut[] = {
//  cfg         input       output      state       
    0x00000000, 0x00000000, 0x00101164, 0x00101166, // LED Blinker
    0x00501168, 0x04001172, 0x040011F2, 0x00A01272  // UART Emulator
};




// No run-time logging task data structure signatures needed in this project




// No task-specific initialization functions




// No task-specific uninitialization functions




/** \brief Performs driver setup dependent hardware initialization
  *
  * This function is called by the internal driver initialization function, \ref scifInit().
  */
static void scifDriverSetupInit(void) {

    // Select SCE clock frequency in active mode
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_AUXSCECLK) = AON_PMCTL_AUXSCECLK_SRC_SCLK_HFDIV2;

    // Set the default power mode
    scifSetSceOpmode(AUX_SYSIF_OPMODEREQ_REQ_A);

    // Initialize task resource dependencies
    scifInitIo(12, AUXIOMODE_OUTPUT | (0 << BI_AUXIOMODE_OUTPUT_DRIVE_STRENGTH), -1, 0);
    scifInitIo(15, AUXIOMODE_OUTPUT, 1, 1);
    scifInitIo(16, AUXIOMODE_INPUT,  1, 0);

} // scifDriverSetupInit




/** \brief Performs driver setup dependent hardware uninitialization
  *
  * This function is called by the internal driver uninitialization function, \ref scifUninit().
  */
static void scifDriverSetupUninit(void) {

    // Uninitialize task resource dependencies
    scifUninitIo(12, -1);
    scifUninitIo(15, 1);
    scifUninitIo(16, 1);

} // scifDriverSetupUninit




/** \brief Re-initializes I/O pins used by the specified tasks
  *
  * It is possible to stop a Sensor Controller task and let the System CPU borrow and operate its I/O
  * pins. For example, the Sensor Controller can operate an SPI interface in one application state while
  * the System CPU with SSI operates the SPI interface in another application state.
  *
  * This function must be called before \ref scifExecuteTasksOnceNbl() or \ref scifStartTasksNbl() if
  * I/O pins belonging to Sensor Controller tasks have been borrowed System CPU peripherals.
  *
  * \param[in]      bvTaskIds
  *     Bit-vector of task IDs for the task I/Os to be re-initialized
  */
void scifReinitTaskIo(uint32_t bvTaskIds) {
    if (bvTaskIds & (1 << SCIF_LED_BLINKER_TASK_ID)) {
        scifReinitIo(12, -1, 0);
    }
    if (bvTaskIds & (1 << SCIF_UART_EMULATOR_TASK_ID)) {
        scifReinitIo(15, 1, 0);
        scifReinitIo(16, 1, 0);
    }
} // scifReinitTaskIo




/// Driver setup data, to be used in the call to \ref scifInit()
const SCIF_DATA_T scifDriverSetup = {
    (volatile SCIF_INT_DATA_T*) 0x400E014C,
    (volatile SCIF_TASK_CTRL_T*) 0x400E015A,
    (volatile uint16_t*) 0x400E013C,
    0x0000,
    sizeof(pAuxRamImage),
    pAuxRamImage,
    pScifTaskDataStructInfoLut,
    pAuxIoIndexToMcuIocfgOffsetLut,
    0x0000,
    24,
    scifDriverSetupInit,
    scifDriverSetupUninit,
    (volatile uint16_t*) NULL,
    (volatile uint16_t*) NULL,
    NULL
};




/** \brief Starts or modifies RTC-based task scheduling tick generation
  *
  * RTC-based tick generation will wake up the Sensor Controller first at the specified value of the RTC
  * and then periodically at the specified interval. The application must call this function after
  * calling \ref scifInit().
  *
  * The application must ensure that:
  * - \a tickStart is not in the past (prefer using \ref scifStartRtcTicksNow() to avoid this)
  * - \a tickPeriod is not too short. RTC ticks will be skipped silently if the Sensor Controller does
  *   not complete its tasks within a single tick interval.
  *
  * \param[in]      tickStart
  *     RTC value when the first tick is generated:
  *     - Bits 31:16 = seconds
  *     - Bits 15:0 = 1/65536 of a second
  * \param[in]      tickPeriod
  *     Interval at which subsequent ticks are generated:
  *     - Bits 31:16 = seconds
  *     - Bits 15:0 = 1/65536 of a second
  */
void scifStartRtcTicks(uint32_t tickStart, uint32_t tickPeriod) {

    // Configure RTC channel 2
    HWREG(AON_RTC_BASE + AON_RTC_O_CH2CMP) = tickStart;
    HWREG(AON_RTC_BASE + AON_RTC_O_CH2CMPINC) = tickPeriod;
    HWREG(AON_RTC_BASE + AON_RTC_O_CHCTL) |= AON_RTC_CHCTL_CH2_EN_M | AON_RTC_CHCTL_CH2_CONT_EN_M;

    // Prevent glitches to the edge detector when enabling the wake-up source
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU0CFG) = AUX_SYSIF_PROGWU0CFG_WU_SRC_AON_RTC_CH2;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU0CFG) = AUX_SYSIF_PROGWU0CFG_WU_SRC_AON_RTC_CH2 | AUX_SYSIF_PROGWU0CFG_EN_M;

} // scifStartRtcTicks




/** \brief Starts or modifies RTC-based task scheduling tick generation
  *
  * RTC-based tick generation will wake up the Sensor Controller first after approximately 128 us and
  * then periodically at the specified interval. The application must call this function after calling
  * \ref scifInit().
  *
  * The application must ensure that \a tickPeriod is not too short. RTC ticks will be skipped silently
  * if the Sensor Controller does not complete its tasks within a single tick interval.
  *
  * \param[in]      tickPeriod
  *     Interval at which subsequent ticks are generated:
  *     - Bits 31:16 = seconds
  *     - Bits 15:0 = 1/65536 of a second
  */
void scifStartRtcTicksNow(uint32_t tickPeriod) {
    uint32_t key, sec, subsec;

    // Read the current RTC value
    key = scifOsalEnterCriticalSection();
    sec = HWREG(AON_RTC_BASE + AON_RTC_O_SEC);
    subsec = HWREG(AON_RTC_BASE + AON_RTC_O_SUBSEC);

    // Start RTC tick generation
    scifStartRtcTicks(((sec << 16) | (subsec >> 16)) + 8, tickPeriod);
    scifOsalLeaveCriticalSection(key);

} // scifStartRtcTicksNow




/** \brief Stops RTC-based task scheduling tick generation
  *
  * The application must call this function before calling \ref scifUninit().
  */
void scifStopRtcTicks(void) {

    // Disable RTC channel 2
    HWREG(AON_RTC_BASE + AON_RTC_O_CHCTL) &= ~(AON_RTC_CHCTL_CH2_EN_M | AON_RTC_CHCTL_CH2_CONT_EN_M);
    HWREG(AON_RTC_BASE + AON_RTC_O_SYNC);

    // Prevent glitches to the edge detector when disabling the wake-up source
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU0CFG) = AUX_SYSIF_PROGWU0CFG_WU_SRC_AON_RTC_CH2;

} // scifStopRtcTicks




// Register aliases (CC13x2/CC26x2 and CC13x4/CC26x4 names are used in the source code)
#ifndef AUX_TIMER01_BASE
    #define AUX_TIMER01_BASE            AUX_TIMER_BASE
#endif
#ifndef AUX_TIMER01_O_T0CTL
    #define AUX_TIMER01_O_T0CTL         AUX_TIMER_O_T0CTL
#endif
#ifndef AUX_TIMER01_O_T0CFG
    #define AUX_TIMER01_O_T0CFG         AUX_TIMER_O_T0CFG
#endif
#ifndef AUX_TIMER01_O_T0TARGET
    #define AUX_TIMER01_O_T0TARGET      AUX_TIMER_O_T0TARGET
#endif
#ifndef AUX_TIMER01_T0CFG_PRE_S
    #define AUX_TIMER01_T0CFG_PRE_S     AUX_TIMER_T0CFG_PRE_S
#endif
#ifndef AUX_TIMER01_T0CFG_RELOAD_M
    #define AUX_TIMER01_T0CFG_RELOAD_M  AUX_TIMER_T0CFG_RELOAD_M
#endif




/** \brief Sets the RX byte timeout
  *
  * The RX timeout starts once a byte has been received. The last received byte is flagged (see
  * \ref scifUartRxGetChar()) when the UART RX line has been idle for the specified period of time.
  *
  * \note This function must only be called while the UART receiver is disabled!
  *
  * \param[in]      rxTimeout
  *     Timeout in half bit periods
  */
void scifUartSetRxTimeout(uint16_t rxTimeout) {
    scifTaskData.uartEmulator.cfg.rxByteTimeout = rxTimeout;
} // scifUartSetRxTimeout




/** \brief Sets the number of required idle bit periods when enabling the UART receiver
  *
  * To prevent enabling RX in the middle of a byte and receiving garbage data, the required idle count
  * should be set to 20 (or more). After calling \c scifUartRxEnable(1), start bit detection is enabled
  * once the RX line has been idle (high) for the specified period of time
  *
  * \note This function must only be called while the UART receiver is disabled!
  *
  * \param[in]      rxEnableReqIdleCount
  *     Required idle time in number of half bit periods
  */
void scifUartSetRxEnableReqIdleCount(uint16_t rxEnableReqIdleCount) {
    scifTaskData.uartEmulator.cfg.rxEnableReqIdleCount = rxEnableReqIdleCount;
} // scifUartSetRxEnableReqIdleCount




/** \brief Enables or disables the UART receiver
  *
  * When enabling, the UART begins detecting start bit after the configurable required idle period (see
  * \ref scifUartSetRxEnableReqIdleCount()).
  *
  * When disabling, the UART receiver will be deactivated within one bit-period. Any ongoing byte
  * reception is aborted. Note that this also prevents RX timeout for the last received byte.
  *
  * \param[in]      rxEnable
  *     True to enable RX, false to disable RX
  */
void scifUartRxEnable(uint16_t rxEnable) {
    scifTaskData.uartEmulator.state.rxEnable = rxEnable;
} // scifUartRxEnable




/** \brief Stops the UART emulator
  *
  * When running the UART emulation task, the execution code runs continuously, and can only be stopped
  * by calling this function. This stops the UART emulator within one bit period.
  *
  * \note This function must only be called while the UART baud generator is active, and baud rate
  *       generation must not be disabled until the stop operation has taken effect.
  */
void scifUartStopEmulator(void) {
    scifTaskData.uartEmulator.state.exit = 1;
} // scifUartStopEmulator




/** \brief Sets the UART baud rate
  *
  * This function must be called to start baud rate generation before or after starting the UART
  * emulation task. This function can be called during operation to change the baud rate on-the-fly.
  *
  * \param[in]      baudRate
  *     The new baud rate (for example 115200 or 9600)
  */
void scifUartSetBaudRate(uint32_t baudRate) {
    uint32_t t0Period;
    uint32_t t0PrescalerExp;

    // Start baud rate generation?
    if (baudRate) {

        // Calculate the AUX timer 0 period
        t0Period = 24000000 / (4 * baudRate);

        // The period must be 65535 clock cycles or less, so adjust the prescaler until it is
        t0PrescalerExp = 0;
        while (t0Period > 65535) {
            t0PrescalerExp += 1;
            t0Period >>= 1;
        }

        // Stop baud rate generation while reconfiguring
        HWREG(AUX_TIMER01_BASE + AUX_TIMER01_O_T0CTL) = 0;

        // Set period and prescaler, and select reload mode
        HWREG(AUX_TIMER01_BASE + AUX_TIMER01_O_T0CFG) = (t0PrescalerExp << AUX_TIMER01_T0CFG_PRE_S) | AUX_TIMER01_T0CFG_RELOAD_M;
        HWREG(AUX_TIMER01_BASE + AUX_TIMER01_O_T0TARGET) = t0Period;

        // Start baud rate generation
        HWREG(AUX_TIMER01_BASE + AUX_TIMER01_O_T0CTL) = 1;

    // Baud rate 0 -> stop baud rate generation
    } else {
        HWREG(AUX_TIMER01_BASE + AUX_TIMER01_O_T0CTL) = 0;
    }

} // scifUartSetBaudRate




/** \brief Sets ALERT event threshold for the RX FIFO
  *
  * \param[in]      threshold
  *     Number of bytes (or higher) in the RX FIFO that triggers ALERT interrupt
  */
void scifUartSetRxFifoThr(uint16_t threshold) {
    scifTaskData.uartEmulator.cfg.alertRxFifoThr = threshold;
} // scifUartSetRxFifoThr




/** \brief Calculates the number of bytes currently stored in the RX FIFO
  *
  * Note that the RX FIFO can only store the configured number of characters minus one. This number is
  * defined by \ref SCIF_UART_RX_FIFO_MAX_COUNT.
  *
  * \return
  *     The number of characters in the RX FIFO, ready to be read
  */
uint32_t scifUartGetRxFifoCount(void) {
    uint16_t rxTail = scifTaskData.uartEmulator.state.rxTail;
    uint16_t rxHead = scifTaskData.uartEmulator.state.rxHead;
    if (rxHead < rxTail) {
        rxHead += sizeof(scifTaskData.uartEmulator.output.pRxBuffer) / sizeof(uint16_t);
    }
    return rxHead - rxTail;
} // scifUartGetRxFifoCount




/** \brief Receives a single character, with flags
  *
  * This function must only be called when there is at least one character in the RX FIFO. Calling this
  * function when the FIFO is empty will cause underflow, without warning.
  *
  * \return
  *     The received character, with flags:
  *     - [15:12] - Reserved
  *     - [11] - RX timeout occurred after receiving this character
  *     - [10] - RX FIFO ran full when receiving this character (this character may be invalid)
  *     - [9] - Break occurred when receiving this character
  *     - [8] - Framing error occurred when receiving this character
  *     - [7:0] - The character value
  */
uint16_t scifUartRxGetChar(void) {

    // Get the character
    uint32_t rxTail = scifTaskData.uartEmulator.state.rxTail;
    uint16_t c = scifTaskData.uartEmulator.output.pRxBuffer[rxTail];

    // Update the TX FIFO head index
    if (++rxTail == (sizeof(scifTaskData.uartEmulator.output.pRxBuffer) / sizeof(uint16_t))) {
        rxTail = 0;
    }
    scifTaskData.uartEmulator.state.rxTail = (uint16_t) rxTail;

    return c;
} // scifUartRxGetChar




/** \brief Receives the specified number of character, without flags
  *
  * This function must only be called with count equal to or less than the current RX FIFO count. Calling
  * this function with too high count will cause underflow, without warning.
  *
  * \param[in,out]  *pBuffer
  *     Pointer to the character destination buffer
  * \param[in]      count
  *     Number of characters to get
  */
void scifUartRxGetChars(char* pBuffer, uint32_t count) {
    int n;

    // For each character ...
    uint32_t rxTail = scifTaskData.uartEmulator.state.rxTail;
    for (n = 0; n < count; n++) {

        // Get it
        pBuffer[n] = (char) scifTaskData.uartEmulator.output.pRxBuffer[rxTail];

        // Update the TX FIFO head index
        if (++rxTail == (sizeof(scifTaskData.uartEmulator.output.pRxBuffer) / sizeof(uint16_t))) {
            rxTail = 0;
        }
    }
    scifTaskData.uartEmulator.state.rxTail = (uint16_t) rxTail;

} // scifUartRxGetChars




/** \brief Receives the specified number of character, with flags
  *
  * This function must only be called with count equal to or less than the current RX FIFO count. Calling
  * this function with too high count will cause underflow, without warning.
  *
  * \param[in,out]  *pBuffer
  *     Pointer to the character destination buffer. Each entry contains the following
  *     - [15:12] - Reserved
  *     - [11] - RX timeout occurred after receiving this character
  *     - [10] - RX FIFO ran full when receiving this character (this character may be invalid)
  *     - [9] - Break occurred when receiving this character
  *     - [8] - Framing error occurred when receiving this character
  *     - [7:0] - The character value
  * \param[in]      count
  *     Number of characters to get
  */
void scifUartRxGetCharsWithFlags(uint16_t* pBuffer, uint32_t count) {
    int n;

    // For each character ...
    uint32_t rxTail = scifTaskData.uartEmulator.state.rxTail;
    for (n = 0; n < count; n++) {

        // Get it
        pBuffer[n] = scifTaskData.uartEmulator.output.pRxBuffer[rxTail];

        // Update the TX FIFO head index
        if (++rxTail == (sizeof(scifTaskData.uartEmulator.output.pRxBuffer) / sizeof(uint16_t))) {
            rxTail = 0;
        }
    }
    scifTaskData.uartEmulator.state.rxTail = (uint16_t) rxTail;

} // scifUartRxGetCharsWithFlags




/** \brief Sets ALERT event threshold for the TX FIFO
  *
  * \param[in]      threshold
  *     Number of bytes (or lower) in the TX FIFO that triggers ALERT interrupt
  */
void scifUartSetTxFifoThr(uint16_t threshold) {
    scifTaskData.uartEmulator.cfg.alertTxFifoThr = threshold;
} // scifUartSetTxFifoThr




/** \brief Calculates the number of bytes currently stored in the TX FIFO
  *
  * The count is decremented when a character's stop bits are transmitted.
  *
  * Note that the TX FIFO can only store the configured number of characters minus one. This number is
  * defined by \ref SCIF_UART_TX_FIFO_MAX_COUNT.
  *
  * \return
  *     The number of characters in the TX FIFO, waiting to be transmitted
  */
uint32_t scifUartGetTxFifoCount(void) {
    uint16_t txTail = scifTaskData.uartEmulator.state.txTail;
    uint16_t txHead = scifTaskData.uartEmulator.state.txHead;
    if (txHead < txTail) {
        txHead += sizeof(scifTaskData.uartEmulator.input.pTxBuffer) / sizeof(uint16_t);
    }
    return txHead - txTail;
} // scifUartGetTxFifoCount




/** \brief Transmits a single character, without delay
  *
  * This function must not be called when the TX FIFO is full. Calling this function when the FIFO is
  * full will cause overflow, without warning. The number of free entries in the FIFO is:
  * \code
  * SCIF_UART_TX_FIFO_MAX_COUNT - scifUartGetTxFifoCount()
  * \endcode
  *
  * \param[in]      c
  *     The character to transmit
  */
void scifUartTxPutChar(char c) {

    // Put the character
    uint32_t txHead = scifTaskData.uartEmulator.state.txHead;
    scifTaskData.uartEmulator.input.pTxBuffer[txHead] = (uint8_t) c;

    // Update the TX FIFO head index
    if (++txHead == (sizeof(scifTaskData.uartEmulator.input.pTxBuffer) / sizeof(uint16_t))) {
        txHead = 0;
    }
    scifTaskData.uartEmulator.state.txHead = (uint16_t) txHead;

} // scifUartTxPutChar




/** \brief Transmits a single character, after the specified delay
  *
  * This function must not be called when the TX FIFO is full. Calling this function when the FIFO is
  * full will cause overflow, without warning. The number of free entries in the FIFO is:
  * \code
  * SCIF_UART_TX_FIFO_MAX_COUNT - scifUartGetTxFifoCount()
  * \endcode
  *
  * \param[in]      c
  *     The character to transmit
  * \param[in]      delay
  *     Delay inserted before this character is transmitted, given in number of bits
  */
void scifUartTxPutCharDelayed(char c, uint8_t delay) {

    // Put the character, with delay
    uint32_t txHead = scifTaskData.uartEmulator.state.txHead;
    scifTaskData.uartEmulator.input.pTxBuffer[txHead] = (delay << 8) | (uint8_t) c;

    // Update the TX FIFO head index
    if (++txHead == (sizeof(scifTaskData.uartEmulator.input.pTxBuffer) / sizeof(uint16_t))) {
        txHead = 0;
    }
    scifTaskData.uartEmulator.state.txHead = (uint16_t) txHead;

} // scifUartTxPutCharDelayed




/** \brief Transmits the specified number of character
  *
  * This function must not be called with count higher than the number of free entries in the TX FIFO.
  * Calling this function with too high count will cause overflow, without warning. The number of free
  * entries in the FIFO is:
  * \code
  * SCIF_UART_TX_FIFO_MAX_COUNT - scifUartGetTxFifoCount()
  * \endcode
  *
  * \param[in,out]  *pBuffer
  *     Pointer to the character source buffer
  * \param[in]      count
  *     Number of characters to put
  */
void scifUartTxPutChars(char* pBuffer, uint32_t count) {
    int n;

    // For each character ...
    uint32_t txHead = scifTaskData.uartEmulator.state.txHead;
    for (n = 0; n < count; n++) {

        // Get it
        scifTaskData.uartEmulator.input.pTxBuffer[txHead] = (uint8_t) pBuffer[n];

        // Update the TX FIFO head index
        if (++txHead == (sizeof(scifTaskData.uartEmulator.input.pTxBuffer) / sizeof(uint16_t))) {
            txHead = 0;
        }
    }
    scifTaskData.uartEmulator.state.txHead = (uint16_t) txHead;

} // scifUartTxPutChars




/** \brief Returns the SCIF UART events that had occurred when the last ALERT interrupt was generated
  *
  * This function does not return any new events until \ref scifUartClearEvents() has been called. Events
  * that have occurred in the meantime are stored in a backlog.
  *
  * The reported events exclude masked-out events (see \ref scifUartSetEventMask()).
  *
  * \return
  *     Bit-vector of the events that have occurred (one or more of the following):
  *     - \ref BV_SCIF_UART_ALERT_RX_FIFO_ABOVE_THR
  *     - \ref BV_SCIF_UART_ALERT_RX_BYTE_TIMEOUT
  *     - \ref BV_SCIF_UART_ALERT_RX_BREAK_OR_FRAMING_ERROR
  *     - \ref BV_SCIF_UART_ALERT_TX_FIFO_BELOW_THR
  */
uint16_t scifUartGetEvents(void) {
    return scifTaskData.uartEmulator.state.alertEvents;
} // scifUartGetEvents




/** \brief Clears the SCIF UART events reported by the last call to \ref scifUartGetEvents()
  *
  * This function must be called to get further event reports through ALERT interrupts.
  */
void scifUartClearEvents(void) {
    scifTaskData.uartEmulator.state.alertEvents = 0x0000;
} // scifUartClearEvents




/** \brief Select which SCIF UART events that shall trigger the ALERT interrupt
  *
  * The ALERT interrupt is generated when one or more of the events included in the mask occur.
  *
  * \param[in]      mask
  *     Bit-vector of the events that shall generate the ALERT interrupt (zero or more of the following):
  *     - \ref BV_SCIF_UART_ALERT_RX_FIFO_ABOVE_THR
  *     - \ref BV_SCIF_UART_ALERT_RX_BYTE_TIMEOUT
  *     - \ref BV_SCIF_UART_ALERT_RX_BREAK_OR_FRAMING_ERROR
  *     - \ref BV_SCIF_UART_ALERT_TX_FIFO_BELOW_THR
  */
void scifUartSetEventMask(uint16_t mask) {

    // Disable all events temporarily to avoid unwanted FIFO events
    scifTaskData.uartEmulator.cfg.alertMask           = mask;
    scifTaskData.uartEmulator.state.alertMask         = 0x0000;

} // scifUartSetEventMask


//@}


// Generated by LT5CD3044HTN at 2026-06-01 15:05:45.188
