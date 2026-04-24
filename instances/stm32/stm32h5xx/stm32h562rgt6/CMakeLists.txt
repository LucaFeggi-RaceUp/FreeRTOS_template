message(STATUS "Configuring STM32H562rgt6 instance")

set(PLATFORM_DEFS
  STM32H562xx
  FREERTOS_MAX_PRIORITIES=7
  FREERTOS_HEAP_BYTES=131072
)

# uppercase
set(STM32_DEVICE
  STM32H562xx
)

# lowercase
set(STM32_device
  stm32h562xx
)

include(${CMAKE_SOURCE_DIR}/instances/stm32/stm32h5xx/CMakeLists.txt)
