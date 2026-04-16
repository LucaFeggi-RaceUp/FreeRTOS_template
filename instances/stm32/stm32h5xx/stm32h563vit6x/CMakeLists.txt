message(STATUS "Configuring STM32H563vit6x instance")

set(PLATFORM_DEFS
  STM32H563xx
  FREERTOS_MAX_PRIORITIES=7
  FREERTOS_HEAP_BYTES=131072
)

# uppercase
set(STM32_DEVICE
  STM32H563xx
)

# lowercase
set(STM32_device
  stm32h563xx
)

include(${CMAKE_SOURCE_DIR}/instances/stm32/stm32h5xx/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/instances/stm32/CMakeLists.txt)

include(${CMAKE_SOURCE_DIR}/instances/stm32/toolchain.cmake)
