# Sensor Controller SDK Documentation

Context files for working with the TI Sensor Controller on CC13xx/CC26xx devices.

# Assembly Reference
https://software-dl.ti.com/lprf/sensor_controller_studio/docs/cc13x2_cc26x2_help/html/assembly_language_reference.html

## Contents

| File | Contents |
|---|---|
| [00_overview.md](00_overview.md) | Architecture, chip families, execution flow, SDK layout |
| [01_sccode_language.md](01_sccode_language.md) | SCCode syntax, data types, task blocks, constants |
| [02_resource_api.md](02_resource_api.md) | All SCCode resource APIs: ADC, I²C, SPI, GPIO, Timers, UART, Comparators, Math, RTL |
| [03_scif_driver_api.md](03_scif_driver_api.md) | System CPU SCIF API: init, task control, alert handling, data access |
| [04_examples_catalog.md](04_examples_catalog.md) | 30+ example projects with patterns and chip support |
| [05_project_setup.md](05_project_setup.md) | Creating projects, generating code, CCS setup, build errors, debugging |

## Quick Start

1. Install **Sensor Controller Studio** and a **SimpleLink SDK** (see `05_project_setup.md`)
2. Copy the framework files from SCS to your project (`scif_framework.c/h`, `scif_osal_none.c/h`)
3. Write your task in a `.scp` file, click **Generate Code** → copies `scif.c` and `scif.h`
4. Write `main.c` using the SCIF API (see `03_scif_driver_api.md`)
5. Build in CCS and flash

## This Project (`adc_poll`)

An ADC interval poller that samples DIO23 at 10 Hz:

```
sensorcontroller/
├── adc_poll.scp          ← Open in Sensor Controller Studio; click Generate Code
├── source/
│   ├── main.c            ← System CPU application
│   ├── scif.c/.h         ← PLACEHOLDER — regenerate from adc_poll.scp
│   ├── scif_framework.c/.h
│   └── scif_osal_none.c/.h
└── ccs/
    └── adc_poll.projectspec   ← Import into CCS via File → Import → CCS Projects
```

**Before building**: open `adc_poll.scp` in SCS, click Generate Code, copy the output `scif.c`/`scif.h` into `source/`.
