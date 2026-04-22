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

- `config/tmpl`: placeholder root configuration, including
  `FreeRTOSConfig.h` and driver id headers. Copy or rename this for the real
  board configuration used by your firmware.
- `config/driver_smoke`: concrete root configuration selected by the on-target
  driver smoke-test presets.
- `instances/host`: host startup and FreeRTOS simulator glue.
- `instances/stm32/stm32h5xx`: shared STM32H5 family setup.
- `lib/drivers`: portable driver interfaces and backend implementations.
- `lib/drivers/instances/host`: host driver backend.
- `lib/drivers/instances/stm32h5xx`: STM32H5 driver backend.
- `lib/drivers/instances/stm32h5xx/config/tmpl`: placeholder STM32H5 driver
  mapping for the template. Copy or rename this for the real board pin and
  peripheral mapping.
- `lib/drivers/instances/stm32h5xx/config/driver_smoke`: STM32H5 driver mapping
  selected by the on-target driver smoke-test preset.
- `app`: application sources linked into the firmware target.
- `tests/on_target/driver_smoke`: selectable on-target smoke-test application for
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

## Using The Template For A Board

The `tmpl` configuration is only a starting point. For a real firmware project,
replace it with a configuration named after the board or product you are
building for. Do not treat `tmpl` as a shared production configuration; board
configuration belongs to the board.

There are two board-specific configuration layers:

- `CONFIG_DIR`: the root configuration directory. It provides
  `FreeRTOSConfig.h` and the logical driver id headers under `driver_ids`.
- `STM32H5XX_DRIVER_CONFIG`: the STM32H5 driver mapping directory under
  `lib/drivers/instances/stm32h5xx/config`. It maps the logical driver ids to
  real STM32 ports, pins, peripheral instances, channels, alternate functions,
  EEPROM layout, and watchdog resources.

The logical ids are the names used by application code. The backend mapping is
what makes those names real on a specific board. For example, if the application
uses `GpioId::STATUS_LED`, then `config/<board>/driver_ids/gpio_id.hpp` must
declare `STATUS_LED`, and
`lib/drivers/instances/stm32h5xx/config/<board>/mapping.hpp` must map
`STATUS_LED` to the actual GPIO port, pin, polarity, and initial state for that
board.

Typical project setup:

1. Choose a board configuration name, for example `my_board`.
2. Copy `config/tmpl` to `config/my_board`.
3. Edit `config/my_board/FreeRTOSConfig.h` for the board's FreeRTOS heap,
   priorities, hooks, and enabled features.
4. Edit the headers in `config/my_board/driver_ids` so they contain the logical
   resources your application uses, such as LEDs, ADC inputs, timers, serial
   ports, CAN buses, EEPROM areas, and watchdogs.
5. Copy `lib/drivers/instances/stm32h5xx/config/tmpl` to
   `lib/drivers/instances/stm32h5xx/config/my_board`.
6. Edit `lib/drivers/instances/stm32h5xx/config/my_board/mapping.hpp` so every
   logical driver id is mapped to the actual STM32H5 peripheral and pins on the
   board schematic.
7. Replace the example application in `app`, or create another application
   directory and select it with `APP_DIR`.
8. Add board-specific CMake presets that select the board configuration.

For example, a board preset should point both configuration layers at the board:

```json
{
  "name": "my-board-debug",
  "displayName": "My Board Debug",
  "inherits": "stm32h563-base",
  "binaryDir": ".build/my-board-debug",
  "cacheVariables": {
    "CONFIG_DIR": "${sourceDir}/config/my_board",
    "STM32H5XX_DRIVER_CONFIG": "my_board",
    "CMAKE_BUILD_TYPE": "Debug"
  }
}
```

This is the configure preset entry. Add matching `buildPresets` and
`workflowPresets` entries following the existing preset pattern if you want to
build it with `cmake --workflow --preset my-board-debug`.

If the board uses the same STM32H563VIT6x MCU variant as the template presets,
it can inherit from `stm32h563-base`. If it uses a different STM32 package or
family, add the matching instance under `instances/stm32` and set `INSTANCE` to
that instance from the preset. The `DRIVERS_INSTANCE` value selects the driver
backend family, such as `host` or `stm32h5xx`.

Keep configuration ownership clear:

- Application behavior belongs in `app` or in the directory selected by
  `APP_DIR`.
- RTOS and logical resource ids belong in `config/<board>`.
- STM32 pin and peripheral bindings belong in
  `lib/drivers/instances/stm32h5xx/config/<board>/mapping.hpp`.
- Startup, linker, clock, and MCU-package details belong in `instances`.
- External dependency code under `third_party` should not be edited as board
  configuration.

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

Build the STM32H563 on-target driver smoke-test target:

```bash
cmake --workflow --preset stm32h563-driver-smoke-debug
```

Available workflows:

- `host-debug`
- `host-release`
- `stm32h563-debug`
- `stm32h563-release`
- `stm32h563-driver-smoke-debug`
- `stm32h563-driver-smoke-release`

The default template presets compile the sources listed in `app/CMakeLists.txt`
and use `config/tmpl` plus the STM32H5 `tmpl` driver mapping. A real board
should use its own presets that select `config/<board>` with `CONFIG_DIR` and
`lib/drivers/instances/stm32h5xx/config/<board>` with
`STM32H5XX_DRIVER_CONFIG`. Alternate applications can be selected with the
`APP_DIR` CMake cache variable.

Build output is written under `.build/<preset>`. The Ninja backend handles
incremental rebuilds and uses its native parallel job scheduling by default. STM32
firmware builds produce `firmware.elf`, `firmware.hex`, `firmware.bin`, and
`firmware.map`; the post-link step also prints the ELF size.

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

## On-Target Driver Smoke Test

The on-target driver smoke test is a firmware application, not a host/unit test.
It uses the same `app_start()` entry point as a normal application and is meant
to be built, flashed, and run on real target hardware.

`on_target` describes where the test runs. The exact chip is selected by the
CMake preset through `INSTANCE`, and the board wiring is selected by
`CONFIG_DIR` plus `STM32H5XX_DRIVER_CONFIG`. The provided
`stm32h563-driver-smoke-*` presets target the STM32H563VIT6x instance and the
`driver_smoke` board mapping. A different board can reuse this test application
when its own configuration provides the same logical driver ids and maps them to
the board's real pins and peripherals.

The smoke-test application creates static FreeRTOS tasks for:

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
