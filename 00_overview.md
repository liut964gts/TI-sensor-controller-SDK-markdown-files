# Sensor Controller SDK — Overview

## What Is the Sensor Controller?

The Sensor Controller (SC) is a dedicated ultra-low-power co-processor embedded in Texas Instruments CC13xx and CC26xx wireless MCUs. It lives in the **AUX power domain** alongside the ADC, comparators, and other analog peripherals. Its purpose is to run small sensing tasks (sample an ADC, poll a GPIO, talk to an I²C sensor) while the main ARM Cortex-M core is in deep sleep, dramatically reducing average current consumption.

```
┌─────────────────────────────────────────────────────┐
│  CC26xx / CC13xx SoC                                │
│                                                     │
│  ┌─────────────────┐    ┌─────────────────────────┐ │
│  │  System CPU     │    │  AUX Domain             │ │
│  │  Cortex-M3/M4   │◄──►│  ┌──────────────────┐   │ │
│  │                 │    │  │ Sensor Controller│   │ │
│  │  (can deep      │    │  │ (always on)      │   │ │
│  │   sleep while   │    │  │                  │   │ │
│  │   SC runs)      │    │  │ 2–4 KB AUX RAM   │   │ │
│  └─────────────────┘    │  └────────┬─────────┘   │ │
│                         │           │             │ │
│                         │  ┌────────▼──────────┐  │ │
│                         │  │ Analog Peripherals│  │ │
│                         │  │ ADC, COMPA/B, I2S │  │ │
│                         │  └───────────────────┘  │ │
│                         └─────────────────────────┘ │
└─────────────────────────────────────────────────────┘
```

## Key Properties

| Property | Value |
|---|---|
| ISA | TI proprietary 16-bit RISC (not ARM) |
| Clock | 24 MHz (driven from AUX oscillator) |
| AUX RAM | 2 KB on CC26x0/CC13x0; 4 KB on CC26x2/CC13x2 |
| Active current | ~0.4 µA when Sensor Controller is running without RF |
| Typical task current | 0.5–4 µA average depending on duty cycle |
| Task programming | SCCode (C-like DSL compiled by Sensor Controller Studio) |
| System CPU communication | Shared AUX RAM + interrupt lines (SWEV0/SWEV1) |

## Chip Family Support

| Family | Devices | AUX RAM | Notes |
|---|---|---|---|
| CC26x0 | CC2640, CC2650 | 2 KB | Original; QFN48 7×7 |
| CC13x0 | CC1310, CC1350 | 2 KB | Sub-GHz RF version |
| CC26x0R2 | CC2640R2 | 2 KB | Minor revision of CC26x0 |
| CC26x2 | CC2642R1F, CC2652R, CC2652RSIP, CC2652P1F, CC2652PSIP, CC2652P7 | 4 KB | Larger AUX RAM; multiprotocol; "P" variants add PA |
| CC13x2 | CC1312R, CC1352R, CC1352P1F3, CC1352P7 | 4 KB | Sub-GHz, larger AUX RAM; "P" variants add PA |
| CC267x | CC2672R3, CC2672P3 | 4 KB | Newer multiprotocol parts |

The shipped `adc_data_logger_launchpad` example targets CC2642R1F by default; its `main.c` `#ifdef` cascade also covers CC1352R/P, CC2652R/P/SIP, and CC2672R/P variants.

## Sensor Controller Studio

Sensor Controller Studio (SCS) is the **required desktop tool** for writing and compiling Sensor Controller firmware. It is installed at:

```
C:\Program Files (x86)\Texas Instruments\Sensor Controller Studio\
```

SCS does the following:
- Provides a GUI editor for SCCode task blocks (Initialize, Execute, Terminate, Event handlers)
- Manages I/O pin mapping and resource allocation
- **Compiles SCCode to AUX RAM bytecode** (this step cannot be done by hand or by GCC/TI CGT)
- **Generates `scif.c` / `scif.h`** — the System CPU driver files containing the compiled image
- Provides a simulator for stepping through SC execution

> **Critical**: You cannot write a valid `pAuxRamImage[]` by hand. You must open the `.scp` project in SCS, click **Generate Code**, and copy the output into your CCS project.

## Execution Flow

```
Power-on
  │
  ▼
System CPU: main()
  ├─ scifInit(&scifDriverSetup)      // load AUX RAM image, register interrupts
  ├─ scifStartRtcTicksNow(period)    // arm AON_RTC channel 2
  ├─ IntMasterEnable()
  └─ scifStartTasksNbl(BV(TASK_ID)) // send start request to SC
         │
         ▼
  [SC runs Initialize block once]
         │
         ▼ (every RTC tick)
  [SC runs Execute block]
  ├─ samples ADC / talks to sensor
  ├─ writes result to output struct in AUX RAM
  ├─ fwGenAlertInterrupt()          // pauses SC, fires ISR on System CPU
  └─ (waits for ACK)
         │
         ▼ (on System CPU)
  taskAlertCallback() [ISR]
  ├─ scifClearAlertIntSource()
  ├─ read scifTaskData.xxx.output.*  // safe: SC is paused
  ├─ g_newSample = true
  └─ scifAckAlertEvents()           // resumes SC
         │
         ▼
  System CPU main loop
  └─ processAdcSample() [thread context]
```

## SDK Directory Layout

```
<SDK_INSTALL_DIR>/
├── source/
│   ├── ti/devices/
│   │   ├── cc26x0/               ← CC2640/CC2650 headers + driverlib
│   │   │   ├── driverlib/
│   │   │   │   └── bin/ccs/driverlib.lib
│   │   │   ├── inc/              ← hw_*.h register headers
│   │   │   ├── linker_files/     ← cc26x0f128.cmd
│   │   │   └── startup_files/   ← ccfg.c, startup_ccs.c
│   │   ├── cc26x0r2/
│   │   ├── cc13x0/
│   │   └── cc26x2/
│   └── ...
└── ...
```

## Sensor Controller Studio Directory Layout

```
C:\Program Files (x86)\Texas Instruments\Sensor Controller Studio\
├── examples/                     ← ~30 ready-to-run example projects (.scp)
│   ├── adc_data_logger/
│   ├── i2c_humidity_sensor/
│   ├── spi_accelerometer/
│   └── ...
├── resources/
│   ├── fw_templates/             ← scif_framework__0.c/.h (copy verbatim)
│   │   └── scif_framework__0.c  ← edit line: replace <<<osal_c_file>>> with scif_osal_none.c
│   ├── osal_defs/               ← scif_osal_none__0.c/.h (copy verbatim)
│   └── resource_defs/           ← .red files defining SCCode API per resource
└── docs/
```

## Communication Model

The System CPU and Sensor Controller share data through **AUX RAM** at `0x400E0000`. The SCIF driver defines C structs that map directly to this memory.

- **Output struct**: SC writes, System CPU reads (after `fwGenAlertInterrupt()` pauses SC)
- **Input struct**: System CPU writes, SC reads (used for configuration)
- **Cfg struct**: System CPU writes before starting task, SC reads in Initialize
- **State struct**: SC writes and reads (internal state between executions)

Synchronization uses two interrupt lines:
- `INT_AON_AUX_SWEV0` (READY): SC has executed Initialize; task control request complete
- `INT_AON_AUX_SWEV1` (ALERT): SC called `fwGenAlertInterrupt()`; AUX RAM is readable
