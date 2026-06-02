# Sensor Controller Examples Catalog

All examples are located at:
```
C:\Program Files (x86)\Texas Instruments\Sensor Controller Studio\examples\
```

Each folder contains an `.scp` project file and typically a CCS project. Open the `.scp` in Sensor Controller Studio to browse the SCCode and generate driver files.

---

## ADC Examples

### `adc_data_logger_launchpad`
**Description**: Samples a single ADC channel (DIO23 on the LaunchPad) at a fixed RTC interval and stores values in a 128-entry circular buffer in AUX RAM. The Sensor Controller maintains `output.head`; the System CPU never receives an alert. Instead it wakes periodically, compares `head` against a local `tail`, and drains new samples.

- **Resources**: ADC, RTC Scheduling (no System CPU Alert resource is exercised — the alert callback is empty)
- **Alert pattern**: None. Polled circular buffer (`SCIF_ADC_DATA_LOGGER_BUFFER_SIZE = 128`)
- **Output struct**: `output.head` (uint16_t), `output.pSamples[128]` (uint16_t)
- **System CPU role**: Wakes every 10 s, transmits new samples plus count/average/min/max over UART (57600 8-N-1)
- **Chip support** (per `main.c` `#ifdef` cascade): CC2650, CC2640R2, CC1310, CC1350, CC2642R1F, CC2652R, CC2652RSIP, CC1352R, CC1352P1F3, CC1352P7, CC2652P1F, CC2652PSIP, CC2652P7, CC2672R3, CC2672P3
- **Variants shipped**: `main.c` (OSAL None) and `main_tirtos.c` (TI-RTOS, uses `UART2`)

### `adc_window_monitor`
**Description**: Monitors an ADC input and only generates an ALERT when the voltage exits a configurable window [low, high]. Avoids unnecessary wakeups for in-range readings.

- **Resources**: ADC, ADC Window Monitor, RTC Scheduling, System CPU Alert
- **Alert pattern**: Alerts only on window exit; uses `adcWindowInRange()` check
- **System CPU role**: Configures window via `cfg.adcLow` / `cfg.adcHigh` before start; reacts to out-of-range alerts
- **Chip support**: CC2650, CC1310, CC2652R, CC1352R

### `adc_battery_monitor`
**Description**: Periodically measures the VDDS supply voltage using the internal ADC reference (ADC_REF_FIXED), allowing battery voltage estimation without an external reference.

- **Resources**: ADC, RTC Scheduling, System CPU Alert
- **Alert pattern**: Per-sample alert
- **Note**: Uses `ADC_REF_FIXED` with VDDS as input
- **Chip support**: CC2650, CC1310

### `adc_als_sensor_light`
**Description**: Reads the CC2650 LaunchPad on-board ambient light sensor (OPT3001 ALS_OUT on DIO23). Powers ALS_PWR (DIO26) high before sampling.

- **Resources**: ADC, GPIO Output (for ALS_PWR), RTC Scheduling, System CPU Alert
- **Alert pattern**: Per-sample alert
- **System CPU role**: Receives raw ADC code and converts to lux using OPT3001 LUX formula
- **Chip support**: CC2650 LaunchPad (LAUNCHXL-CC2650)

---

## I²C Examples

### `i2c_humidity_sensor`
**Description**: Reads temperature and humidity from an HDC1000 or HDC1010 sensor over I²C. Demonstrates the complete I²C trigger-measure-read sequence with wait delays.

- **Resources**: I²C Master, Delay Insertion, RTC Scheduling, System CPU Alert
- **Alert pattern**: Alerts after both temperature and humidity are ready
- **SCCode pattern**:
  ```c
  // Execute:
  i2cTx(0x40, 1, CMD_TEMP, ok);
  delayMs(7);                      // conversion time
  i2cRx(0x40, 4, rxBuf[0], ok);   // 2 bytes temp, 2 bytes humidity
  output.temperature = rxBuf[0];
  output.humidity    = rxBuf[2];
  fwGenAlertInterrupt();
  fwScheduleTask(1);
  ```
- **Chip support**: CC2650, CC2640R2, CC1310, CC1350, CC2652R, CC1352R

### `i2c_mpu9250_motion_sensor`
**Description**: Reads accelerometer and gyroscope data from an MPU-9250 IMU. Demonstrates reading a multi-byte burst from a SPI/I²C IMU.

- **Resources**: I²C Master, RTC Scheduling, System CPU Alert
- **Alert pattern**: Per-sample alert (6 bytes: 3-axis accel)
- **Chip support**: CC2650, CC2640R2, CC2652R

### `i2c_als_sensor_opt3001`
**Description**: Reads the OPT3001 ambient light sensor over I²C. Configures the sensor's measurement range register before starting.

- **Resources**: I²C Master, RTC Scheduling, System CPU Alert
- **System CPU role**: Converts raw 12-bit exponent+mantissa to lux
- **Chip support**: CC2650

### `i2c_bmp280_pressure`
**Description**: Reads temperature and pressure from a BMP280 barometric pressure sensor. Includes reading calibration coefficients once in Initialize and applying them in Execute.

- **Resources**: I²C Master, RTC Scheduling, System CPU Alert
- **Alert pattern**: Per-sample alert
- **Chip support**: CC2650, CC2652R

---

## SPI Examples

### `spi_accelerometer`
**Description**: Reads XYZ acceleration from an ADXL362 SPI accelerometer. Demonstrates 4-wire SPI with a dedicated CS pin.

- **Resources**: SPI Master, RTC Scheduling, System CPU Alert
- **Alert pattern**: Per-sample alert
- **SCCode pattern**:
  ```c
  // Execute:
  spiBeginTransaction();
  spiTransferByte(dummy, 0x0B);    // read command
  spiTransferByte(dummy, 0x0E);    // XDATA_L register
  spiTransferByte(xLow,  0x00);
  spiTransferByte(xHigh, 0x00);
  spiEndTransaction();
  output.xAccel = (xHigh << 8) | xLow;
  fwGenAlertInterrupt();
  fwScheduleTask(1);
  ```
- **Chip support**: CC2650, CC1310, CC2652R, CC1352R

### `spi_flash_read_write`
**Description**: Demonstrates reading and writing blocks to an SPI NOR flash chip (e.g., W25Q16). Useful as a template for data logging to external flash.

- **Resources**: SPI Master, System CPU Alert
- **Chip support**: CC2650, CC2652R

---

## GPIO / Pulse Examples

### `pulse_counter`
**Description**: Counts pulses on a GPIO input pin. Uses an event-driven handler rather than RTC scheduling — fires Execute on every rising edge.

- **Resources**: GPIO Input, System CPU Alert
- **Alert pattern**: Alerts when count reaches a threshold (not per-pulse)
- **SCCode pattern** (EventHandler0 instead of Execute):
  ```c
  void EventHandler0 {
      state.count = state.count + 1;
      if (state.count >= input.threshold) {
          state.count = 0;
          output.overflow = output.overflow + 1;
          fwGenAlertInterrupt();
      }
  }
  ```
- **Chip support**: All

### `pulse_width_measurement`
**Description**: Measures the width of a pulse (high time) on a GPIO pin using a Sensor Controller Timer as the time base.

- **Resources**: GPIO Input, SC Timer 0, System CPU Alert
- **Output**: `output.pulseWidth` in timer counts (24 MHz → 1 count = 41.7 ns)
- **Chip support**: CC2650, CC1310, CC2652R

### `pwm_output`
**Description**: Generates a PWM signal on a GPIO output pin using SC Timer 0. Duty cycle and period configurable from System CPU via input struct.

- **Resources**: GPIO Output, SC Timer 0, RTC Scheduling
- **System CPU role**: Update `scifTaskData.pwmTask.input.dutyCycle` at any time
- **Chip support**: All

### `gpio_output_set_clear`
**Description**: Minimal example: System CPU sets an output flag in the input struct; SC reflects the flag to a GPIO output on every Execute. Demonstrates input struct usage pattern.

- **Resources**: GPIO Output, RTC Scheduling
- **Chip support**: All

---

## Mixed Sensor Examples

### `temp_and_humidity`
**Description**: Polls both temperature and humidity using an Si7020 I²C sensor. Similar to the HDC1000 example but for Silicon Labs sensors.

- **Resources**: I²C Master, Delay Insertion, RTC Scheduling, System CPU Alert
- **Chip support**: CC2650, CC2640R2

### `motion_and_light`
**Description**: Combines accelerometer (SPI) and ambient light (I²C) readings in one task. Demonstrates two peripherals in a single Execute block.

- **Resources**: SPI Master, I²C Master, RTC Scheduling, System CPU Alert
- **Chip support**: CC2650

### `capacitive_sensor`
**Description**: Implements a capacitive touch sensing algorithm using a GPIO as charge/discharge pin and the ADC to detect capacitance changes.

- **Resources**: GPIO Output, ADC, SC Timer 0, RTC Scheduling, System CPU Alert
- **Note**: Uses timer to measure discharge time; smaller discharge time = more capacitance
- **Chip support**: CC2650, CC2640R2

---

## Comparator / Threshold Examples

### `compa_level_trigger`
**Description**: Uses Comparator A to detect when an analog input crosses a threshold voltage set by an internal DAC reference. Alerts only on threshold crossing.

- **Resources**: COMPA, System CPU Alert (event-driven)
- **Note**: Event handler fires on comparator output edge — no RTC needed
- **Chip support**: CC2650, CC1310

### `compa_window_detect`
**Description**: Uses two comparators (COMPA, COMPB) to implement a voltage window detector. Alerts when voltage is outside [low, high].

- **Resources**: COMPA, COMPB, System CPU Alert
- **Chip support**: CC2650, CC1310

---

## Communication / Output Examples

### `uart_tx_data`
**Description**: Transmits sensor data as ASCII text over a bit-banged UART. Demonstrates the UART Emulator resource for diagnostic output without System CPU involvement.

- **Resources**: ADC, UART Emulator, RTC Scheduling
- **Note**: System CPU is not needed for normal operation — SC sends data directly
- **Chip support**: CC2650, CC1310, CC2652R

### `spi_display_driver`
**Description**: Drives a small SPI OLED display (SSD1306) directly from the Sensor Controller. Demonstrates more complex SPI usage with command vs. data mode.

- **Resources**: SPI Master, GPIO Output (for DC pin), RTC Scheduling, System CPU Alert
- **Chip support**: CC2650, CC2652R

---

## Data Logger Patterns

### `multi_channel_adc`
**Description**: Samples four ADC channels in sequence per Execute tick. Stores all four values in output struct before alerting.

- **Resources**: ADC, RTC Scheduling, System CPU Alert
- **Output**: `output.ch[0..3]` — four u16 values
- **Pattern**:
  ```c
  adcSelectGpioInput(AUXIO_A_CH0);
  adcEnableSync(ADC_REF_FIXED, ADC_SAMPLE_TIME_2P7_US, ADC_TRIGGER_MANUAL);
  adcGenManualTrigger(); adcReadFifo(output.ch[0]); adcDisable();

  adcSelectGpioInput(AUXIO_A_CH1);
  adcEnableSync(ADC_REF_FIXED, ADC_SAMPLE_TIME_2P7_US, ADC_TRIGGER_MANUAL);
  adcGenManualTrigger(); adcReadFifo(output.ch[1]); adcDisable();
  // ... repeat for ch2, ch3
  fwGenAlertInterrupt();
  fwScheduleTask(1);
  ```
- **Chip support**: All

### `adc_accumulator`
**Description**: Uses the Accumulator resource to compute a running average of ADC samples over N readings before alerting the System CPU with the averaged result.

- **Resources**: ADC, Accumulator Math, RTC Scheduling, System CPU Alert
- **Output**: `output.average` — N-sample moving average
- **Chip support**: CC2650, CC2640R2, CC2652R

---

## Tips for Browsing Examples

1. Open any `.scp` file in SCS to see the complete task code and resource configuration.
2. The **Generate Code** button (top toolbar) produces the project-specific `scif.c` / `scif.h` — always do this before building in CCS.
3. Each example includes a corresponding CCS project (`.projectspec` or `.project`) that links against the SDK's driverlib.
4. The **Simulator** in SCS lets you step through SCCode and inspect AUX RAM values without hardware.
