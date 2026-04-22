set(_drivers_instance_dir "${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/../instance_layout.cmake")

list(REMOVE_ITEM DRIVERS_INSTANCE_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/flex_can.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/multi_can.cpp"
)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(STM32CubeH5 REQUIRED MODULE)

set(_stm32h5_default_device "STM32H563xx")
if(DEFINED STM32_DEVICE)
  set(_stm32h5_default_device "${STM32_DEVICE}")
endif()

set(STM32H5_DEVICE "${_stm32h5_default_device}" CACHE STRING
  "STM32H5 device macro used to compile the stm32h5xx drivers instance"
)

set(STM32H5XX_DRIVER_CONFIG "tmpl" CACHE STRING
  "STM32H5xx driver mapping configuration"
)

set(_stm32h5xx_driver_config_dir
  "${CMAKE_CURRENT_LIST_DIR}/config/${STM32H5XX_DRIVER_CONFIG}"
)

if(NOT EXISTS "${_stm32h5xx_driver_config_dir}/mapping.hpp")
  message(FATAL_ERROR
    "STM32H5XX_DRIVER_CONFIG='${STM32H5XX_DRIVER_CONFIG}' does not provide "
    "${_stm32h5xx_driver_config_dir}/mapping.hpp"
  )
endif()

set(_stm32h5_hal_sources
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_adc.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_adc_ex.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_cortex.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_dma.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_dma_ex.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_exti.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_fdcan.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_flash.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_flash_ex.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_gpio.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_iwdg.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pcd.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pcd_ex.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pwr.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_pwr_ex.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_rcc.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_rcc_ex.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_tim.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_hal_uart.c"
  "${STM32CubeH5_HAL_SOURCE_DIR}/stm32h5xx_ll_usb.c"
)

if(TARGET device_hal)
  list(APPEND DRIVERS_INSTANCE_PRIVATE_LINK_LIBRARIES
    device_hal
  )
else()
  list(APPEND DRIVERS_INSTANCE_SOURCES
    "${STM32CubeH5_SYSTEM_SOURCE}"
    ${_stm32h5_hal_sources}
  )
endif()

set(_stm32h5_eeprom_emul_root
  "${CMAKE_CURRENT_LIST_DIR}/../../../../third_party/STM32/STM32H5/stm32-util-eeprom-emulation"
)

if(NOT EXISTS "${_stm32h5_eeprom_emul_root}/core/eeprom_emul_core.c")
  message(FATAL_ERROR
    "drivers stm32h5xx: missing ST EEPROM emulation submodule at "
    "${_stm32h5_eeprom_emul_root}. Run git submodule update --init --recursive."
  )
endif()

list(APPEND DRIVERS_INSTANCE_SOURCES
  "${_stm32h5_eeprom_emul_root}/core/eeprom_emul_core.c"
  "${_stm32h5_eeprom_emul_root}/core/algo/eeprom_algo_flitf.c"
)

set(_stm32h5_usbx_root
  "${STM32CubeH5_ROOT_DIR}/Middlewares/ST/usbx"
)

file(GLOB _stm32h5_usbx_core_sources
  "${_stm32h5_usbx_root}/common/core/src/*.c"
)
list(SORT _stm32h5_usbx_core_sources)

file(GLOB _stm32h5_usbx_cdc_sources
  "${_stm32h5_usbx_root}/common/usbx_device_classes/src/ux_device_class_cdc_acm*.c"
)
list(SORT _stm32h5_usbx_cdc_sources)

file(GLOB _stm32h5_usbx_dcd_sources
  "${_stm32h5_usbx_root}/common/usbx_stm32_device_controllers/*.c"
)
list(SORT _stm32h5_usbx_dcd_sources)

list(APPEND DRIVERS_INSTANCE_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/usb/usb_cdc_standalone.c"
  "${CMAKE_CURRENT_LIST_DIR}/usb/usb_cdc.c"
  "${CMAKE_CURRENT_LIST_DIR}/usb/ux_standalone_support.c"
  ${_stm32h5_usbx_core_sources}
  ${_stm32h5_usbx_cdc_sources}
  ${_stm32h5_usbx_dcd_sources}
)

list(APPEND DRIVERS_INSTANCE_PUBLIC_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/config"
  "${STM32CubeH5_HAL_INCLUDE_DIR}"
  "${STM32CubeH5_CMSIS_INCLUDE_DIR}"
  "${STM32CubeH5_CMSIS_DEVICE_INCLUDE_DIR}"
)

list(APPEND DRIVERS_INSTANCE_PRIVATE_INCLUDE_DIRS
  "${_stm32h5xx_driver_config_dir}"
  "${_stm32h5_eeprom_emul_root}/core"
  "${_stm32h5_eeprom_emul_root}/core/algo"
  "${_stm32h5_eeprom_emul_root}/interface_flash"
  "${_stm32h5_eeprom_emul_root}/interface_crc"
  "${_stm32h5_usbx_root}/common/core/inc"
  "${_stm32h5_usbx_root}/common/usbx_device_classes/inc"
  "${_stm32h5_usbx_root}/common/usbx_stm32_device_controllers"
  "${_stm32h5_usbx_root}/ports/cortex_m4/gnu/inc"
  "${CMAKE_CURRENT_LIST_DIR}/usb"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(_stm32h5xx_probe_source "${CMAKE_CURRENT_BINARY_DIR}/stm32h5xx_toolchain_probe.cpp")
  set(_stm32h5xx_system_include_flags)
  file(WRITE "${_stm32h5xx_probe_source}" "\n")

  execute_process(
    COMMAND "${CMAKE_CXX_COMPILER}" -E -x c++ -v "${_stm32h5xx_probe_source}"
    OUTPUT_QUIET
    ERROR_VARIABLE _stm32h5xx_compiler_stderr
    RESULT_VARIABLE _stm32h5xx_compiler_result
  )

  if(_stm32h5xx_compiler_result EQUAL 0)
    string(REPLACE "\r" "" _stm32h5xx_compiler_stderr "${_stm32h5xx_compiler_stderr}")
    string(REPLACE "\n" ";" _stm32h5xx_compiler_lines "${_stm32h5xx_compiler_stderr}")
    set(_stm32h5xx_include_candidates)
    set(_stm32h5xx_collect_includes FALSE)

    foreach(_stm32h5xx_compiler_line IN LISTS _stm32h5xx_compiler_lines)
      string(STRIP "${_stm32h5xx_compiler_line}" _stm32h5xx_compiler_line)

      if(_stm32h5xx_compiler_line STREQUAL "#include <...> search starts here:")
        set(_stm32h5xx_collect_includes TRUE)
        continue()
      endif()

      if(_stm32h5xx_compiler_line STREQUAL "End of search list.")
        break()
      endif()

      if(_stm32h5xx_collect_includes AND _stm32h5xx_compiler_line)
        list(APPEND _stm32h5xx_include_candidates "${_stm32h5xx_compiler_line}")
      endif()
    endforeach()

    foreach(_stm32h5xx_include_dir IN LISTS _stm32h5xx_include_candidates)
      if(IS_DIRECTORY "${_stm32h5xx_include_dir}")
        list(APPEND _stm32h5xx_system_include_flags "-isystem${_stm32h5xx_include_dir}")
      endif()
    endforeach()
  endif()
endif()

list(APPEND DRIVERS_INSTANCE_COMPILE_DEFINITIONS
  USE_HAL_DRIVER
  ${STM32H5_DEVICE}
  UX_INCLUDE_USER_DEFINE_FILE
)

if(TARGET freertos_kernel)
  list(APPEND DRIVERS_INSTANCE_PRIVATE_LINK_LIBRARIES
    freertos_kernel
  )
endif()

list(APPEND DRIVERS_INSTANCE_COMPILE_OPTIONS
  ${_stm32h5xx_system_include_flags}
)
