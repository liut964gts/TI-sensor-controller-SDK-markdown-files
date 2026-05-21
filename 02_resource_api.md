# SCCode Resource API Reference

Resources are "plug-in" blocks of SCCode API that Sensor Controller Studio adds to a project. Each resource provides a set of functions callable from task code blocks. Resources are selected in the SCS project GUI.

---

## ADC — Analog-to-Digital Converter

Samples an analog voltage on an AUX IO pin via the CC26xx on-chip 12-bit SAR ADC.

```c
// Select analog input pin
adcSelectGpioInput(u16 auxIoIndex);

// Enable ADC in synchronous mode
adcEnableSync(u16 refSource, u16 sampleTime, u16 triggerSource);

// Enable ADC in asynchronous mode (lower power for longer sample times)
adcEnableAsync(u16 refSource, u16 sampleTime);

// Generate a manual trigger (when triggerSource == ADC_TRIGGER_MANUAL)
adcGenManualTrigger();

// Wait for conversion to complete and read result into variable
adcReadFifo(u16& result);

// Read FIFO without waiting (check adcFifoEmpty() first)
adcReadFifoCheckForEmpty(u16& result);

// Flush the FIFO (discard pending samples)
adcFlushFifo();

// Check if FIFO is empty (returns 0 or 1)
adcFifoEmpty(bool& empty);

// Disable the ADC (required between samples to save power)
adcDisable();
```

**Typical pattern (single sample per Execute):**
```c
adcEnableSync(ADC_REF_FIXED, ADC_SAMPLE_TIME_2P7_US, ADC_TRIGGER_MANUAL);
adcGenManualTrigger();
adcReadFifo(output.adcValue);
adcDisable();
```

---

## ADC Window Monitor

Compares each ADC sample against a window [low, high] and alerts only when the value is outside the window. Minimizes wakeups for threshold detection.

```c
// Configure window — only generates alert when value exits [low, high]
adcSetWindowConfig(u16 adcLow, u16 adcHigh);

// Enable window comparison (configure before adcEnableSync)
adcEnableWindowComparator();

// Read whether the last sample was inside (1) or outside (0) the window
adcWindowInRange(bool& inRange);
```

**Pattern:** Configure once in Initialize, check `adcWindowInRange` in Execute before alerting.

---

## GPIO Input

Reads one or more digital input pins.

```c
// Configure pin as digital input
gpioSetInput(u16 auxIoIndex, s16 pull);  // pull: -1=float, 0=down, 1=up

// Read pin value (0 or 1)
gpioGetInput(u16 auxIoIndex, bool& value);

// Read all AUX IO pins at once as a 16-bit bitmask
gpioGetInputBitmask(u16& bitmask);
```

---

## GPIO Output

Drives one or more digital output pins.

```c
// Configure pin as output
gpioSetOutput(u16 auxIoIndex, u16 initValue);   // initValue: 0 or 1

// Set pin high
gpioSetPin(u16 auxIoIndex);

// Set pin low
gpioClearPin(u16 auxIoIndex);

// Toggle pin
gpioTogglePin(u16 auxIoIndex);

// Set multiple output pins via bitmask
gpioSetOutputBitmask(u16 setBitmask, u16 clrBitmask);
```

---

## GPIO Input/Output (Bidirectional)

Combines GPIO input and output. Configure direction per-pin.

```c
// Set direction: 0=input, 1=output
gpioSetDirection(u16 auxIoIndex, u16 direction);

// Read and write use same functions as GPIO Input / GPIO Output above
```

---

## I²C Master

Bit-bangs I²C using two AUX IO pins (SCL, SDA).

```c
// Initialize I2C with selected SCL/SDA pins
i2cInit(u16 sclAuxIoIndex, u16 sdaAuxIoIndex);

// Write N bytes to device address
// Returns 0 on success, non-zero on NACK/error
i2cTx(u16 address, u16 byteCount, u16& pData, bool& success);

// Read N bytes from device address
i2cRx(u16 address, u16 byteCount, u16& pData, bool& success);

// Combined write + read (register read pattern)
i2cTxRx(u16 address, u16 txByteCount, u16& pTxData,
         u16 rxByteCount, u16& pRxData, bool& success);
```

**Pattern (read 2 bytes from register 0x00 of device at address 0x40):**
```c
u16 txBuf;
u16 rxBuf[2];
bool ok;
txBuf = 0x0000;   // register address
i2cTxRx(0x40, 1, txBuf, 2, rxBuf[0], ok);
output.rawMsb = rxBuf[0];
output.rawLsb = rxBuf[1];
```

---

## SPI Master (3-wire or 4-wire)

Communicates with SPI peripherals using up to 4 AUX IO pins (SCLK, MOSI, MISO, CSN).

```c
// Configure SPI: clock polarity, phase, bit order, clock frequency
spiInit(u16 sclkAuxIoIndex, u16 mosiAuxIoIndex, u16 misoAuxIoIndex,
        u16 csnAuxIoIndex, u16 clockHz, u16 cpol, u16 cpha, u16 msbFirst);

// Assert CS (drive low)
spiBeginTransaction();

// Release CS (drive high)
spiEndTransaction();

// Transfer N bits: transmit txData, receive into rxData
spiTransferByte(u16& rxData, u16 txData);

// Transfer 16 bits
spiTransferWord(u16& rxData, u16 txData);
```

---

## UART Emulator (TX only)

Bit-bangs UART output using one AUX IO pin.

```c
// Configure baud rate and pin
uartInit(u16 txAuxIoIndex, u16 baudRate);

// Send one byte
uartTxByte(u16 byte);

// Send a string (u16 array, null-terminated)
uartTxString(u16& pStr);
```

Common baud rates: 9600, 19200, 38400, 57600, 115200.

---

## Sensor Controller Timer 0 / Timer 1

16-bit timers running from the 24 MHz AUX clock (or prescaled). Used for PWM, pulse measurement, or triggering ADC.

```c
// Timer 0
timer0Init(u16 period, u16 prescaler);   // period in timer clocks
timer0Start();
timer0Stop();
timer0GetCount(u16& count);
timer0SetPeriod(u16 period);
timer0GetOverflowFlag(bool& overflow);
timer0ClearOverflowFlag();

// Timer 1 — same API with timer1Xxx prefix
```

**PWM pattern (50% duty cycle on a GPIO):**
```c
// Initialize block:
timer0Init(24000, 0);   // 1 ms period at 24 MHz
timer0Start();

// Execute block (toggle pin on every overflow):
bool ovf;
timer0GetOverflowFlag(ovf);
if (ovf) {
    timer0ClearOverflowFlag();
    gpioTogglePin(AUXIO_O_PWM_OUT);
}
```

---

## Sensor Controller Timer 2

High-resolution 32-bit timer (CC26x2/CC13x2 only). Supports capture, compare, and PWM.

```c
timer2Init();
timer2Start();
timer2Stop();
timer2GetCount(u32& count);
timer2SetCompare(u16 channel, u32 compareValue);
timer2GetCaptureValue(u16 channel, u32& captureValue);
```

---

## COMPA — Analog Comparator A

Compares two analog signals. Result is 1 if V+ > V−.

```c
// Configure comparator inputs (AUXIO indices or internal references)
compaInit(u16 posAuxIoIndex, u16 negSource);   // negSource: AUXIO or constant

// Read comparator output
compaGetOutput(bool& result);

// Disable
compaDisable();
```

---

## COMPB — Analog Comparator B

Similar to COMPA; independent second comparator. Same API with `compb` prefix.

---

## Math / Logic (bit operations)

Provides 32-bit operations that are otherwise slow on the 16-bit SC:

```c
// 32-bit arithmetic
mathAdd32(u32& result, u32 a, u32 b);
mathSub32(u32& result, u32 a, u32 b);
mathMul32(u32& result, u32 a, u32 b);

// Bit manipulation
mathBitCount(u16& count, u16 value);   // popcount
mathReverseBits(u16& result, u16 value);
mathCLZ(u16& count, u16 value);        // count leading zeros
```

---

## Accumulator Math

Maintains a running accumulator (sum) and computes moving averages.

```c
// Initialize accumulator buffer of length N
accumInit(u16 length);

// Push a new sample; returns running sum
accumPush(u16 sample, u32& sum);

// Push and compute average
accumPushGetAvg(u16 sample, u16& avg);
```

---

## RTC-Based Execution Scheduling

Enables scheduling SC task executions on AON_RTC channel 2. Generated into `scif.c` as System CPU helper functions.

**SCCode side:** `fwScheduleTask(1)` inside the task.

**System CPU side (generated functions in `scif.c`):**
```c
// Start scheduling with an explicit start time
void scifStartRtcTicks(uint32_t tickStart, uint32_t tickPeriod);

// Start scheduling ~128 µs from now (most common)
void scifStartRtcTicksNow(uint32_t tickPeriod);

// Stop scheduling (disable AON_RTC channel 2)
void scifStopRtcTicks(void);
```

`tickPeriod` is a 16.16 fixed-point seconds value: `0x00010000 * Hz`.

---

## System CPU Alert

Enables `fwGenAlertInterrupt()` in SCCode. The SC pauses after calling it, fires `INT_AON_AUX_SWEV1` on the System CPU, and waits.

**System CPU handling:**
```c
void taskAlertCallback(void) {
    scifClearAlertIntSource();          // clear the AUX_EVCTL source
    // ... read scifTaskData.xxx.output.*  (AUX RAM stable here)
    scifAckAlertEvents();               // resumes SC
}
```

---

## Delay Insertion

Inserts busy-wait delays in SCCode. Useful for timing in I²C/SPI bit-banging or settling time after pin changes.

```c
// Delay N microseconds (blocking)
delayUs(u16 us);

// Delay N milliseconds (blocking)
delayMs(u16 ms);
```

---

## AON Domain Access

Reads AON (Always-On) domain registers directly from SCCode. Primarily used to read the RTC counter for timestamping.

```c
// Read current AON_RTC seconds counter
aonGetRtcSec(u16& seconds);

// Read AON_RTC subseconds (fractional, 16-bit)
aonGetRtcSubsec(u16& subsec);
```

---

## Run-Time Logging (RTL)

Enables streaming debug data from SC to System CPU without stopping execution. Used with the SCS simulator or a UART display tool.

```c
// Log a u16 value with format code (for SCS to decode)
fwRtlLogU16(u16 formatCode, u16 value);

// Log two u16 values
fwRtlLogU16x2(u16 formatCode, u16 v0, u16 v1);

// Log a u32 value
fwRtlLogU32(u16 formatCode, u32 value);
```

**System CPU side:** Use the generated `scifOsalRtlLog...` functions and display with SCS or UART.

---

## I/O Initialization Helpers (scif_framework)

Used internally by `scif.c` resource init/uninit. You rarely call these directly; they are called by `scifInit()` and `scifUninit()`.

```c
// Configure a pin as analog/input/output
void scifInitIo(uint32_t auxIoIndex, uint32_t ioMode, int32_t pull, uint32_t initValue);

// Release a pin back to floating
void scifUninitIo(uint32_t auxIoIndex, int32_t pull);

// Re-initialize after System CPU borrowed the pin
void scifReinitIo(uint32_t auxIoIndex, int32_t pull, uint32_t driveStrength);
```
