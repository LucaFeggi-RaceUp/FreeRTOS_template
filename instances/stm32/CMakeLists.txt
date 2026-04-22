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
