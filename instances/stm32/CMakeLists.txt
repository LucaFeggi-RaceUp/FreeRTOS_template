# STM32 instances - Base template
# Child CMakeLists.txt must define:
#   - HAL_DEVICE_SOURCES: HAL source files
#   - HAL_DEVICE_INCLUDE_DIRS: HAL include directories
#   - CMSIS_DEVICE_SOURCES: CMSIS device sources
#   - CMSIS_DEVICE_INCLUDE_DIRS: CMSIS include directories
#   - CMSIS_GENERIC_INCLUDE_DIRS: CMSIS generic includes directories
#   - STARTUP_SCRIPT: Device startup file (assembly)
#   - PLATFORM_DEFS: Platform-specific compile definitions

message(STATUS "Configuring STM32 instance")

# Create CMSIS device library
add_library(cmsis_device STATIC)

target_sources(cmsis_device PRIVATE
  ${CMSIS_DEVICE_SOURCES}
)

target_include_directories(cmsis_device PUBLIC
  ${CMSIS_DEVICE_INCLUDE_DIRS}
  ${CMSIS_GENERIC_INCLUDE_DIRS}
  ${HAL_DEVICE_INCLUDE_DIRS}
)

target_compile_definitions(cmsis_device PUBLIC
  ${PLATFORM_DEFS}
)

if(TARGET platform_options)
  target_link_libraries(cmsis_device PRIVATE
    platform_options
  )
endif()

# Create HAL library
add_library(device_hal STATIC)

target_sources(device_hal PRIVATE
  ${HAL_DEVICE_SOURCES}
)

target_include_directories(device_hal PUBLIC
  ${HAL_DEVICE_INCLUDE_DIRS}
)

target_compile_definitions(device_hal PUBLIC
  ${PLATFORM_DEFS}
)

target_link_libraries(device_hal PUBLIC
  cmsis_device
  freertos_config
)

if(TARGET platform_options)
  target_link_libraries(device_hal PRIVATE
    platform_options
  )
endif()

# Define additional sources and libraries for firmware
# Preserve any sources already collected by family CMakeLists (e.g., family-specific files)
list(PREPEND FIRMWARE_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/syscalls.cpp
  ${STARTUP_SCRIPT}
)

set(FIRMWARE_LINK_LIBRARIES
  c
  m
  nosys
  device_hal
)

function(platform_finalize_firmware_target target_name)
  get_target_property(_platform_output_name "${target_name}" OUTPUT_NAME)
  if(NOT _platform_output_name OR _platform_output_name STREQUAL "_platform_output_name-NOTFOUND")
    set(_platform_output_name "${target_name}")
  endif()

  set(_platform_firmware_hex "${CMAKE_CURRENT_BINARY_DIR}/${_platform_output_name}.hex")
  set(_platform_firmware_bin "${CMAKE_CURRENT_BINARY_DIR}/${_platform_output_name}.bin")

  add_custom_command(TARGET "${target_name}" POST_BUILD
    BYPRODUCTS
      "${_platform_firmware_hex}"
      "${_platform_firmware_bin}"
    COMMAND "${CMAKE_OBJCOPY}" -O ihex
      $<TARGET_FILE:${target_name}>
      "${_platform_firmware_hex}"
    COMMAND "${CMAKE_OBJCOPY}" -O binary
      $<TARGET_FILE:${target_name}>
      "${_platform_firmware_bin}"
    COMMENT "${_platform_output_name}.hex and ${_platform_output_name}.bin generated as well."
    VERBATIM
  )

  if(CMAKE_SIZE)
    add_custom_command(TARGET "${target_name}" POST_BUILD
      COMMAND "${CMAKE_SIZE}" $<TARGET_FILE:${target_name}>
      COMMENT "${_platform_output_name}.elf size"
      VERBATIM
    )
  endif()
endfunction()
