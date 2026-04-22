# FreeRTOS STM32H5 Template

This repository is a CMake firmware template for FreeRTOS projects that need a
portable driver layer, a host build, and an STM32H563 firmware build.

The template exists to provide a clean starting point for MCU firmware based on
FreeRTOS without tying application code to one device from the beginning. It
keeps the device setup, build rules, and low-level peripheral access behind a
small project structure so the same application logic can be built for a local
host environment and for the target MCU.

Its core goal is portability. Application code should use the driver interfaces
in `lib/drivers/include` when it needs to run on both host and firmware targets.
Device-specific APIs such as STM32 HAL and CMSIS remain available when needed,
but using them directly makes that code specific to the corresponding instance.
The current firmware implementation targets STM32H563 devices; support for new
MCU families should be added through new `instances` entries and matching driver
backends.

The template provides:

- Host builds for local development on Windows and Linux.
- STM32H563 firmware builds.
- FreeRTOS kernel integration.
- STM32CubeH5 HAL and CMSIS integration.
- Driver interfaces for GPIO, ADC, PWM, hardware timers, serial, CAN, flash,
  EEPROM, and watchdog.

Application code lives in `app`. Platform startup, linker scripts, toolchain
setup, and MCU-specific boot code live in `instances`.

The repository is organized so `instances` owns target selection and device
startup, `lib/drivers` owns the portable peripheral abstraction layer, and
`third_party` contains external dependencies such as FreeRTOS, CMSIS, and
STM32Cube libraries.

## Repository Layout

- `config/tmpl`: default template root configuration, including
  `FreeRTOSConfig.h` and driver id headers.
- `config/driver_smoke`: root configuration selected by the board smoke-test
  presets.
- `instances/host`: host startup and FreeRTOS simulator glue.
- `instances/stm32/stm32h5xx`: shared STM32H5 family setup.
- `lib/drivers`: portable driver interfaces and backend implementations.
- `lib/drivers/instances/host`: host driver backend.
- `lib/drivers/instances/stm32h5xx`: STM32H5 driver backend.
- `lib/drivers/instances/stm32h5xx/config/tmpl`: default STM32H5 driver
  mapping for the template.
- `lib/drivers/instances/stm32h5xx/config/driver_smoke`: STM32H5 driver mapping
  selected by the board smoke-test preset.
- `app`: application sources linked into the firmware target.
- `tests/board/driver_smoke`: selectable board smoke-test application for
  FreeRTOS, USB reporting, CAN, ADC, GPIO, and EEPROM bring-up.
- `third_party`: external dependencies tracked as git submodules.

## Dependencies

Install these tools before building:

- CMake 3.20 or newer.
- Ninja.
- A host C and C++ compiler.
- `arm-none-eabi-gcc` for STM32 firmware builds.
- STM32CubeProgrammer, OpenOCD, or a debugger-specific flashing tool for STM32
  targets.

The repository uses these submodules:

- `third_party/FreeRTOS-LTS`
- `third_party/expected`
- `third_party/STM32/STM32H5/STM32CubeH5`
- `third_party/STM32/STM32H5/stm32-util-eeprom-emulation`

FDCAN support comes from STM32CubeH5 through `stm32h5xx_hal_fdcan.c`. A separate
`FDCAN_Driver` submodule is not needed unless the STM32 HAL-backed CAN backend
is intentionally replaced.

## Cloning

Clone with submodules:

```bash
git clone --recurse-submodules <repo-url>
```

For an existing clone, initialize or update the submodules with:

```bash
git submodule update --init --recursive
```

## Building

List the available presets:

```bash
cmake --list-presets
```

Build the host target:

```bash
cmake --workflow --preset host-debug
```

Build the STM32H563 target:

```bash
cmake --workflow --preset stm32h563-debug
```

Build the STM32H563 board driver smoke-test target:

```bash
cmake --workflow --preset stm32h563-driver-smoke-debug
```

Available workflows:

- `host-debug`
- `host-driver-smoke-debug`
- `host-release`
- `stm32h563-debug`
- `stm32h563-driver-smoke-debug`
- `stm32h563-release`

The default build compiles the sources listed in `app/CMakeLists.txt` and uses
`config/tmpl` plus the STM32H5 `tmpl` driver mapping. Alternate applications can
be selected with the `APP_DIR` CMake cache variable. Root configuration is
selected with `CONFIG_DIR`; STM32H5 driver mapping is selected with
`STM32H5XX_DRIVER_CONFIG`.

## Application Code

The default application entry point is `app_start()` in `app/app.cpp`. Platform
startup initializes the minimal driver set needed by the template app, calls
`app_start()`, and starts the FreeRTOS scheduler.

Use `app_start()` for application-level peripheral configuration, task creation,
and initialization that should happen before the scheduler runs. Keep this code
on the portable driver interfaces where possible so it can continue to build for
both host and STM32 instances.

When creating a project from the template:

1. Replace the example logic in `app/app.cpp`.
2. Add new application source files under `app`.
3. Register new source files in `app/CMakeLists.txt`.
4. Use driver interfaces from `lib/drivers/include` for code that should remain
   portable across host and STM32.
5. Use HAL or CMSIS directly only in code that is intentionally STM32-specific.

Driver ids and `FreeRTOSConfig.h` are selected from the root `CONFIG_DIR`.
STM32H5 pin and peripheral mapping is selected from
`lib/drivers/instances/stm32h5xx/config/<STM32H5XX_DRIVER_CONFIG>/mapping.hpp`.

## Board Driver Smoke Test

The board smoke test is a firmware application that uses the same `app_start()`
entry point as a normal application. It creates static FreeRTOS tasks for:

- LED heartbeat on `GpioId::LED_E3`.
- A FreeRTOS queue/timer self-test.
- ADC sampling from `AdcId::POT_0` every 100 ms.
- USB CDC text reporting.
- Classic 11-bit CAN status frames on `M_canId::CAN_1` and `M_canId::CAN_2`.
- EEPROM persistence validation through a boot counter in `EepromId::EEPROM_0`.

The current default ADC smoke mapping is `POT_0 -> ADC1/PA0/ADC_CHANNEL_0`.
Change the `POT_0` entry in
`lib/drivers/instances/stm32h5xx/config/driver_smoke/mapping.hpp` to the real
potentiometer pin before using the ADC test on your board.

CAN smoke-test frame IDs:

- `0x100`: heartbeat and FreeRTOS counters.
- `0x111`: CAN1 receive echo.
- `0x121`: CAN2 receive echo.
- `0x130`: ADC sample report.
- `0x140`: EEPROM boot-counter report.

DBC files are not part of this template. Add DBC generation and generated CAN
sources in the firmware project that is created from the template.

## STM32H563 Setup

The STM32 target is selected through the `INSTANCE` CMake cache variable. The
provided presets set this automatically to `stm32h563vit6x`.

System clock and HAL initialization are owned by the STM32H5 common driver startup.
The default STM32 `main.cpp` stays small: it starts the common, timer, and
serial drivers, calls `app_start()`, and starts the scheduler.

USB CDC descriptors use neutral template defaults. Override
`RU_USB_MANUFACTURER_STRING` and `RU_USB_PRODUCT_STRING` through compile
definitions when creating a product firmware.

## Flashing

Flashing is not automated by this template. For STM32 targets, use
STM32CubeProgrammer, OpenOCD, or the debugger integration used by your
development environment.

## Future Improvements

- Add a typed runtime control path for drivers that need to change behavior
  after initialization. For example, an ADC driver could accept a configuration
  or command that switches between single-shot polling and continuous DMA
  sampling without exposing STM32 HAL details to application code.

  This should be modeled as drivers accepting explicit commands or
  configuration objects rather than drivers inheriting from a command base
  class. Commands are requests sent to a driver; the driver remains the owner of
  its state, validation, stop/reconfigure/start sequence, and backend-specific
  behavior.

- Include only the strictly necessary submodules; avoid importing entire upstream projects such as
  FreeRTOS-LTS or STM32CubeH5.


## Current Limitations

- The host backend is useful for local execution, but it is not a hardware
  simulator.
- Flashing and debug probe setup are intentionally left to the project using the
  template.
