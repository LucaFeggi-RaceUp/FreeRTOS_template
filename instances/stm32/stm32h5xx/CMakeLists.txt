# STM32H5xx family configuration

message(STATUS "Configuring STM32H5xx family")

set(MCPU
  cortex-m33
)

set(FREERTOS_PORT_DIR
  ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM33_NTZ/non_secure
)

set(FREERTOS_KERNEL_SOURCES
  ${FREERTOS_KERNEL_DIR}/list.c
  ${FREERTOS_KERNEL_DIR}/queue.c
  ${FREERTOS_KERNEL_DIR}/tasks.c
  ${FREERTOS_KERNEL_DIR}/timers.c
  ${FREERTOS_KERNEL_DIR}/event_groups.c
  ${FREERTOS_KERNEL_DIR}/stream_buffer.c
)

set(FREERTOS_PORT_SOURCES
  ${FREERTOS_PORT_DIR}/port.c
)

# Conditionally add portasm.c if it exists
if(EXISTS ${FREERTOS_PORT_DIR}/portasm.c)
  list(APPEND FREERTOS_PORT_SOURCES ${FREERTOS_PORT_DIR}/portasm.c)
endif()

set(FREERTOS_PORT_INCLUDE
  ${FREERTOS_PORT_DIR}
  ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM33/secure
  ${FREERTOS_KERNEL_DIR}/portable/ARMv8M/secure/context
)

set(FREERTOS_HEAP_FILE
  ${FREERTOS_KERNEL_DIR}/portable/MemMang/heap_4.c
)

list(PREPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/cmake"
)
find_package(STM32CubeH5 REQUIRED MODULE)

# CMSIS Device
set(CMSIS_DEVICE_INCLUDE_DIRS
  ${STM32CubeH5_CMSIS_DEVICE_INCLUDE_DIR}
)

set(CMSIS_GENERIC_INCLUDE_DIRS
  ${STM32CubeH5_CMSIS_INCLUDE_DIR}
)

set(CMSIS_DEVICE_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/Src/system_stm32h5xx.c
)

# STM32H5xx HAL
set(HAL_DEVICE_INCLUDE_DIRS
  ${STM32CubeH5_HAL_INCLUDE_DIR}
  ${CMSIS_DEVICE_INCLUDE_DIRS}
  ${CMSIS_GENERIC_INCLUDE_DIRS}
  ${CMAKE_CURRENT_LIST_DIR}/Inc
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/config
)

set(HAL_DEVICE_SOURCES
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_adc.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_adc_ex.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_cortex.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_dma.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_dma_ex.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_exti.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_fdcan.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_flash.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_flash_ex.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_gpio.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_iwdg.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pcd.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pcd_ex.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pwr.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pwr_ex.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_rcc.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_rcc_ex.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_tim.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_uart.c
  ${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_ll_usb.c
)

# Platform definitions
list(APPEND PLATFORM_DEFS
  USE_HAL_DRIVER
  FREERTOS_CPU_CLOCK_HZ=250000000
  FREERTOS_TICK_RATE_HZ=1000
  configUSE_TICK_HOOK=1    # DISABLE if not interested in using HAL tick functions
  # ARM CM33 port requirements
  configENABLE_FPU=1
  configENABLE_MPU=0
  configENABLE_TRUSTZONE=0
  configPRIO_BITS=4
  configLIBRARY_LOWEST_INTERRUPT_PRIORITY=15
  configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5
  # Precompute to avoid shell/CMake tokenization issues
  configMAX_SYSCALL_INTERRUPT_PRIORITY=80     # (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
)


set(INSTANCE_MAIN
  ${CMAKE_SOURCE_DIR}/instances/${INSTANCE}/main.cpp
)

set(PLATFORM_MAIN
  ${CMAKE_CURRENT_LIST_DIR}/Src/stm32h5xx_hal_msp.c
  ${CMAKE_CURRENT_LIST_DIR}/Src/stm32h5xx_it.c
)

if(NOT FIRMWARE_STANDALONE_NO_FREERTOS)
  list(APPEND PLATFORM_MAIN
    ${CMAKE_CURRENT_LIST_DIR}/Src/freertos_impl.c
  )
endif()

if(EXISTS ${INSTANCE_MAIN})
  list(PREPEND PLATFORM_MAIN ${INSTANCE_MAIN})
else()
  list(PREPEND PLATFORM_MAIN ${CMAKE_CURRENT_LIST_DIR}/Src/main.cpp)
endif()

set(PLATFORM_MAIN_INCDIR
  ${CMAKE_CURRENT_LIST_DIR}/Inc
)

set(FREERTOS_KERNEL_FORCE_INCLUDE
  freertos_stm32_config.h
)

set(DRIVER_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/common.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/adc.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/can.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/flash_memory.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/gpio.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/i2c.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/pwm.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/serial.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/spi.cpp
  ${CMAKE_SOURCE_DIR}/lib/drivers/instances/stm32h5xx/timer.cpp
)

set(LINKER_SCRIPT_FOLDER
  ${CMAKE_CURRENT_LIST_DIR}/linker/
)

set(STARTUP_SCRIPT
  ${STM32CubeH5_STARTUP_DIR}/startup_${STM32_device}.s
)
