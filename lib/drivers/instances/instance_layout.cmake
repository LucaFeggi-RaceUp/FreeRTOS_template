file(GLOB DRIVERS_INSTANCE_SOURCES
  "${_drivers_instance_dir}/*.cpp"
  "${_drivers_instance_dir}/opaque/*.cpp"
  "${_drivers_instance_dir}/utils/*.cpp"
)
list(SORT DRIVERS_INSTANCE_SOURCES)

function(_drivers_id_list_has_entries out_var id_header macro_name)
  set(_has_entries FALSE)
  set(_id_file "${DRIVER_IDS_INCLUDE_DIR}/${id_header}")

  if(EXISTS "${_id_file}")
    file(STRINGS "${_id_file}" _id_lines)
    set(_in_target_macro FALSE)

    foreach(_id_line IN LISTS _id_lines)
      if(_id_line MATCHES "^[ \t]*#[ \t]*define[ \t]+${macro_name}\\(X\\)(.*)$")
        set(_in_target_macro TRUE)
        string(REGEX REPLACE
          "^[ \t]*#[ \t]*define[ \t]+${macro_name}\\(X\\)[ \t]*"
          ""
          _id_list_body
          "${_id_line}"
        )
      elseif(NOT _in_target_macro)
        continue()
      else()
        set(_id_list_body "${_id_line}")
      endif()

      if(_id_list_body MATCHES "X[ \t]*\\(")
        set(_has_entries TRUE)
      endif()

      if(NOT _id_line MATCHES "\\\\[ \t]*$")
        break()
      endif()
    endforeach()
  endif()

  set(${out_var} "${_has_entries}" PARENT_SCOPE)
endfunction()

function(_drivers_remove_sources_if_id_list_empty id_header macro_name)
  _drivers_id_list_has_entries(_has_entries "${id_header}" "${macro_name}")

  if(NOT _has_entries)
    foreach(_source IN LISTS ARGN)
      list(REMOVE_ITEM DRIVERS_INSTANCE_SOURCES "${_drivers_instance_dir}/${_source}")
    endforeach()

    set(DRIVERS_INSTANCE_SOURCES "${DRIVERS_INSTANCE_SOURCES}" PARENT_SCOPE)
  endif()
endfunction()

_drivers_remove_sources_if_id_list_empty(
  "adc_id.hpp"
  "ADC_LIST"
  "adc.cpp"
  "opaque/opaque_adc.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "gpio_id.hpp"
  "GPIO_LIST"
  "gpio.cpp"
  "opaque/opaque_gpio.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "pwm_id.hpp"
  "PWM_LIST"
  "pwm.cpp"
  "opaque/opaque_pwm.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "timer_id.hpp"
  "TIMER_LIST"
  "timer.cpp"
  "opaque/opaque_timer.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "serial_id.hpp"
  "SERIAL_LIST"
  "serial.cpp"
  "opaque/opaque_serial.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "nv_memory_id.hpp"
  "NV_MEMORY_LIST"
  "nv_memory.cpp"
  "opaque/opaque_nv_memory.cpp"
  "opaque/opaque_nv_memory_eeprom.cpp"
  "opaque/opaque_nv_memory_flash_region.cpp"
  "eeprom_itf_crc_stm32h5.cpp"
  "eeprom_itf_flash_stm32h5.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "watchdog_id.hpp"
  "WATCHDOG_LIST"
  "watchdog.cpp"
  "opaque/opaque_watchdog.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "can_id.hpp"
  "M_CAN_LIST"
  "m_can.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "can_id.hpp"
  "BX_CAN_LIST"
  "bx_can.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "can_id.hpp"
  "FLEX_CAN_LIST"
  "flex_can.cpp"
)

_drivers_remove_sources_if_id_list_empty(
  "can_id.hpp"
  "MULTI_CAN_LIST"
  "multi_can.cpp"
)

_drivers_id_list_has_entries(_drivers_has_m_can "can_id.hpp" "M_CAN_LIST")
_drivers_id_list_has_entries(_drivers_has_bx_can "can_id.hpp" "BX_CAN_LIST")

if(NOT _drivers_has_m_can AND NOT _drivers_has_bx_can)
  list(REMOVE_ITEM DRIVERS_INSTANCE_SOURCES "${_drivers_instance_dir}/can_internal.cpp")
endif()

file(GLOB DRIVERS_INSTANCE_PUBLIC_HEADERS
  "${_drivers_root_dir}/include/*.hpp"
)
list(SORT DRIVERS_INSTANCE_PUBLIC_HEADERS)

file(GLOB DRIVERS_INSTANCE_OPAQUE_HEADERS
  "${_drivers_instance_dir}/opaque/*.hpp"
)
list(SORT DRIVERS_INSTANCE_OPAQUE_HEADERS)

set(DRIVERS_INSTANCE_PUBLIC_INCLUDE_DIRS
  "${_drivers_root_dir}/include"
  "${DRIVER_IDS_INCLUDE_DIR}"
  "${_drivers_instance_dir}/opaque"
)

set(DRIVERS_INSTANCE_PRIVATE_INCLUDE_DIRS
  "${_drivers_instance_dir}"
)
