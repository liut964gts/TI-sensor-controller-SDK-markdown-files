# SCCode Language Reference

SCCode is a C-like domain-specific language used to program the Sensor Controller. It is compiled by Sensor Controller Studio (SCS) — the output is a binary AUX RAM image embedded in the generated `scif.c`.

## Data Types

| Type | Size | Range | Notes |
|---|---|---|---|
| `u8` | 8-bit | 0–255 | Unsigned byte |
| `u16` | 16-bit | 0–65535 | Native SC word size |
| `s16` | 16-bit | −32768–32767 | Signed |
| `u32` | 32-bit | 0–4294967295 | Two registers |
| `s32` | 32-bit | signed | Two registers |
| `bool` | 16-bit | 0 or 1 | Stored as u16 |

All AUX RAM structs must be **16-bit aligned** (use `#pragma pack(push, 2)` on the System CPU side).

## Task Structure

Each task has up to four code blocks:

```c
// Runs once when the task is started (scifStartTasksNbl)
// Use to configure pins, set initial state, schedule first execution
void Initialize {
    adcSelectGpioInput(AUXIO_A_MY_PIN);
    fwScheduleTask(1);          // schedule execute in 1 RTC tick
}

// Runs on every scheduled execution
void Execute {
    adcEnableSync(ADC_REF_FIXED, ADC_SAMPLE_TIME_2P7_US, ADC_TRIGGER_MANUAL);
    adcGenManualTrigger();
    adcReadFifo(output.adcValue);
    adcDisable();
    fwGenAlertInterrupt();      // notify System CPU
    fwScheduleTask(1);          // schedule next execution
}

// Runs once when the task is stopped (scifStopTasksNbl)
void Terminate {
    // release resources, unconfigure pins
}

// Optional: event-driven execution instead of RTC-based
void EventHandler0 {
    // triggered by a GPIO edge, timer overflow, etc.
}
```

## Task Data Structures

Four named structures hold data shared between System CPU and Sensor Controller:

```
cfg     – written by System CPU before start, read by SC in Initialize
input   – written by System CPU at any time, read by SC in Execute
output  – written by SC, read by System CPU after ALERT
state   – SC internal; persists between Execute calls
```

Defined in the SCS project GUI. The generated `scif.h` contains matching C structs.

## Framework Functions (fwXxx)

These are built into every SC firmware image.

### Scheduling

```c
// Schedule this task to run N RTC ticks from now (N >= 1)
fwScheduleTask(u16 nTicks);

// Schedule this task relative to the last execution time (drift-free)
fwScheduleTaskPeriodic(u16 nTicks);
```

### Alert / Interrupt

```c
// Pause SC and raise ALERT interrupt on System CPU.
// SC stays paused until System CPU calls scifAckAlertEvents().
// AUX RAM is safe to read/write during this pause.
fwGenAlertInterrupt();
```

### Run-Time Logging (debug)

```c
// Log a u16 value tagged with a format code (RTL resource must be enabled)
fwRtlLogU16(u16 formatCode, u16 value);
```

## SCCode Operators and Expressions

SCCode supports a subset of C operators:

- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Assignment: `=`, `+=`, `-=`, `&=`, `|=`, `^=`
- Conditional: `if`, `else`, `while`, `for`, `break`, `continue`

No pointers. No dynamic memory allocation. No function calls to user-defined functions (only resource API calls).

## I/O Pin Constants

In SCS, analog pins are given symbolic names. The generated `scif.h` defines constants like:

```c
// Example: project uses DIO23 as "SENSOR_INPUT"
// SCS generates in the .scp:
//   #define AUXIO_A_SENSOR_INPUT  8  // AUXIO index 8 = DIO23
```

The AUXIO index is used in all SCCode I/O calls. The mapping from AUXIO index to physical DIO depends on the chip package and is stored in `pAuxIoIndexToMcuIocfgOffsetLut[]` in `scif.c`.

### CC2650 QFN48 7×7 AUXIO ↔ DIO Mapping

| AUXIO | DIO | AUXIO | DIO |
|---|---|---|---|
| 0 | 0 | 8 | 23 |
| 1 | 1 | 9 | 24 |
| 2 | 2 | 10 | 25 |
| 3 | 3 | 11 | 26 |
| 4 | 4 | 12 | 27 |
| 5 | 5 | 13 | 28 |
| 6 | 6 | 14 | 29 |
| 7 | 7 | 15 | 30 |

## Constants Reference

### ADC Reference Voltages

| Constant | Voltage |
|---|---|
| `ADC_REF_FIXED` | 4.3 V (internal fixed reference) |
| `ADC_REF_VDDS` | VDDS (battery voltage) |

### ADC Sample Times

| Constant | Time |
|---|---|
| `ADC_SAMPLE_TIME_2P7_US` | 2.7 µs |
| `ADC_SAMPLE_TIME_5P3_US` | 5.3 µs |
| `ADC_SAMPLE_TIME_10P6_US` | 10.6 µs |
| `ADC_SAMPLE_TIME_21P3_US` | 21.3 µs |
| `ADC_SAMPLE_TIME_42P6_US` | 42.6 µs |
| `ADC_SAMPLE_TIME_85P3_US` | 85.3 µs |
| `ADC_SAMPLE_TIME_170_US` | 170 µs |
| `ADC_SAMPLE_TIME_341_US` | 341 µs |
| `ADC_SAMPLE_TIME_682_US` | 682 µs |
| `ADC_SAMPLE_TIME_1P37_MS` | 1.37 ms |
| `ADC_SAMPLE_TIME_2P73_MS` | 2.73 ms |
| `ADC_SAMPLE_TIME_5P46_MS` | 5.46 ms |

### ADC Trigger

| Constant | Meaning |
|---|---|
| `ADC_TRIGGER_MANUAL` | Manual trigger via `adcGenManualTrigger()` |
| `ADC_TRIGGER_TIMER0` | Triggered by Sensor Controller Timer 0 |
| `ADC_TRIGGER_TIMER1` | Triggered by Sensor Controller Timer 1 |
| `ADC_TRIGGER_TIMER2` | Triggered by Sensor Controller Timer 2 |

### GPIO Pull

| Value | Meaning |
|---|---|
| `-1` | No pull (float) |
| `0` | Pull-down |
| `1` | Pull-up |

### AUXIO Mode

| Constant | Meaning |
|---|---|
| `AUXIOMODE_INPUT` | Digital input |
| `AUXIOMODE_OUTPUT` | Digital output |
| `AUXIOMODE_ANALOG` | Analog (disconnects digital buffer) |
| `AUXIOMODE_OPEN_DRAIN` | Open-drain output |
| `AUXIOMODE_OPEN_SOURCE` | Open-source output |

## RTC Tick Period Format

The AON_RTC tick period is a **16.16 fixed-point** number of seconds:

```
tickPeriod = 0x00010000 * Hz_rate

Examples:
  1 Hz  → 0x00010000   (every 1 second)
  2 Hz  → 0x00020000
  8 Hz  → 0x00080000
  10 Hz → 0x000A0000   (every 100 ms)
  100 Hz→ 0x00640000
  128 Hz→ 0x00800000
  0.5 Hz→ 0x00008000
```
