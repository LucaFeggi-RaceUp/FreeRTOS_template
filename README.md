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
- Driver interfaces for GPIO, ADC, PWM, timers, serial, CAN, flash, EEPROM, and
  watchdog.

Application code lives in `src`. Platform startup, linker scripts, toolchain
setup, and MCU-specific boot code live in `instances`.

The repository is organized so `instances` owns target selection and device
startup, `lib/drivers` owns the portable peripheral abstraction layer, and
`third_party` contains external dependencies such as FreeRTOS, CMSIS, and
STM32Cube libraries.

## Repository Layout

- `include`: shared configuration headers, including `FreeRTOSConfig.h` and
  driver id headers.
- `instances/host`: host startup and FreeRTOS simulator glue.
- `instances/stm32/stm32h5xx`: shared STM32H5 family setup.
- `lib/drivers`: portable driver interfaces and backend implementations.
- `lib/drivers/instances/host`: host driver backend.
- `lib/drivers/instances/stm32h5xx`: STM32H5 driver backend.
- `src`: application sources linked into the firmware target.
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

Available workflows:

- `host-debug`
- `host-release`
- `stm32h563-debug`
- `stm32h563-release`

The build compiles the sources listed in `src/CMakeLists.txt`. Keep that file
synchronized when adding or removing application files.

## Application Code

The default application entry point is `app_start()` in `src/app.cpp`. Platform
startup initializes the minimal driver set needed by the template app, calls
`app_start()`, and starts the FreeRTOS scheduler.

Use `app_start()` for application-level peripheral configuration, task creation,
and initialization that should happen before the scheduler runs. Keep this code
on the portable driver interfaces where possible so it can continue to build for
both host and STM32 instances.

When creating a project from the template:

1. Replace the example logic in `src/app.cpp`.
2. Add new application source files under `src`.
3. Register new source files in `src/CMakeLists.txt`.
4. Use driver interfaces from `lib/drivers/include` for code that should remain
   portable across host and STM32.
5. Use HAL or CMSIS directly only in code that is intentionally STM32-specific.

Driver ids are configured in `include/driver_ids`. STM32H5 pin and peripheral
mapping is configured in `lib/drivers/instances/stm32h5xx/mapping.hpp`.

DBC files are not part of this template. Add DBC generation and generated CAN
sources in the firmware project that is created from the template.

## STM32H563 Setup

The STM32 target is selected through the `INSTANCE` CMake cache variable. The
provided presets set this automatically to `stm32h563vit6x`.

Clock and HAL initialization are owned by the STM32H5 common driver startup.
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

## Current Limitations

- The host backend is useful for local execution, but it is not a hardware
  simulator.
- Flashing and debug probe setup are intentionally left to the project using the
  template.
