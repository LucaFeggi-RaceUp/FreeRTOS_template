cmake_minimum_required(VERSION 3.20)

## Instance selection --------

if(NOT DEFINED INSTANCE)
  message(FATAL_ERROR "Use -DINSTANCE=<host|stm32h563vit6x>")
endif()

# Check if instance is an STM32 target
string(REGEX MATCH "^stm32" IS_STM32 "${INSTANCE}")

if(IS_STM32)
  # Extract first 7 chars + "xx" to determine family (e.g., stm32h5 -> stm32h5xx)
  string(SUBSTRING ${INSTANCE} 0 7 FAMILY_PREFIX)
  set(FAMILY_DIR ${FAMILY_PREFIX}xx)
  set(INSTANCE_DIR ${CMAKE_SOURCE_DIR}/instances/stm32/${FAMILY_DIR}/${INSTANCE})
else()
  # Host instance.
  set(INSTANCE_DIR ${CMAKE_SOURCE_DIR}/instances/${INSTANCE})
endif()

if(NOT EXISTS ${INSTANCE_DIR}/CMakeLists.txt)
  message(FATAL_ERROR "Unknown INSTANCE='${INSTANCE}' - CMakeLists.txt not found")
endif()

if(IS_STM32)
  project(multitarget_freertos LANGUAGES C CXX ASM)
else()
  project(multitarget_freertos LANGUAGES C CXX)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

set(FREERTOS_KERNEL_DIR
  "${CMAKE_SOURCE_DIR}/third_party/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel"
  CACHE PATH "FreeRTOS kernel directory"
)

set(CONFIG_DIR
  "${CMAKE_SOURCE_DIR}/config/tmpl"
  CACHE PATH "Selected root configuration directory"
)

set(DRIVER_IDS_INCLUDE_DIR "${CONFIG_DIR}/driver_ids")
set(FREERTOS_CONFIG_INCLUDE_DIR "${CONFIG_DIR}")

if(NOT EXISTS "${FREERTOS_CONFIG_INCLUDE_DIR}/FreeRTOSConfig.h")
  message(FATAL_ERROR
    "CONFIG_DIR='${CONFIG_DIR}' does not contain FreeRTOSConfig.h"
  )
endif()

if(NOT IS_DIRECTORY "${DRIVER_IDS_INCLUDE_DIR}")
  message(FATAL_ERROR
    "CONFIG_DIR='${CONFIG_DIR}' does not contain a driver_ids directory"
  )
endif()

if(NOT DEFINED DRIVERS_INSTANCE)
  set(DRIVERS_INSTANCE "${INSTANCE}")
endif()

set(DRIVERS_INSTANCE
  "${DRIVERS_INSTANCE}"
  CACHE STRING "Driver backend instance used by lib/drivers"
)

set(APP_DIR
  "${CMAKE_SOURCE_DIR}/app"
  CACHE PATH "Directory containing the application CMakeLists.txt"
)

include(${INSTANCE_DIR}/CMakeLists.txt)


## Addition of targets -----

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE
  ${FREERTOS_CONFIG_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)
if(DEFINED PLATFORM_MAIN_INCDIR)
  target_include_directories(freertos_config INTERFACE
    ${PLATFORM_MAIN_INCDIR}
  )
endif()

target_compile_definitions(freertos_config INTERFACE
  ${PLATFORM_DEFS}
)

add_library(freertos_kernel STATIC
  ${FREERTOS_KERNEL_SOURCES}
  ${FREERTOS_PORT_SOURCES}
  ${FREERTOS_HEAP_FILE}
)

target_include_directories(freertos_kernel PUBLIC
  ${FREERTOS_KERNEL_DIR}/include
  ${FREERTOS_PORT_INCLUDE}
)

target_link_libraries(freertos_kernel PUBLIC
  freertos_config
)

if(DEFINED FREERTOS_KERNEL_FORCE_INCLUDE)
  target_compile_options(freertos_kernel PRIVATE
    -include
    ${FREERTOS_KERNEL_FORCE_INCLUDE}
  )
endif()

add_subdirectory(lib/drivers)

if(NOT EXISTS "${APP_DIR}/CMakeLists.txt")
  message(FATAL_ERROR
    "APP_DIR='${APP_DIR}' does not contain a CMakeLists.txt"
  )
endif()

add_subdirectory("${APP_DIR}" "${CMAKE_BINARY_DIR}/app")

if(DEFINED FREERTOS_KERNEL_FORCE_INCLUDE)
  target_compile_options(app PRIVATE
    -include
    ${FREERTOS_KERNEL_FORCE_INCLUDE}
  )
endif()

# Link any platform-specific libraries to drivers
if(DEFINED DRIVERS_LINK_LIBRARIES)
  target_link_libraries(drivers PUBLIC
    ${DRIVERS_LINK_LIBRARIES}
  )
endif()

add_library(firmware_objects OBJECT
  ${FIRMWARE_SOURCES}
  ${PLATFORM_MAIN}
)

if(DEFINED PLATFORM_MAIN_INCDIR)
  target_include_directories(firmware_objects PUBLIC
    ${PLATFORM_MAIN_INCDIR}
  )
endif()

target_link_libraries(firmware_objects PRIVATE
  ${FIRMWARE_LINK_LIBRARIES}
)

if(NOT FIRMWARE_STANDALONE_NO_FREERTOS)
  target_link_libraries(firmware_objects PRIVATE
    app
    freertos_kernel
  )
endif()

if(DEFINED FREERTOS_KERNEL_FORCE_INCLUDE)
  target_compile_options(firmware_objects PRIVATE
    -include
    ${FREERTOS_KERNEL_FORCE_INCLUDE}
  )
endif()

add_executable(firmware EXCLUDE_FROM_ALL
  $<TARGET_OBJECTS:firmware_objects>
)

if(DEFINED PLATFORM_MAIN_INCDIR)
  target_include_directories(firmware PUBLIC
    ${PLATFORM_MAIN_INCDIR}
  )
endif()

if(NOT FIRMWARE_STANDALONE_NO_FREERTOS)
  target_link_libraries(firmware PRIVATE
    app
    freertos_kernel
  )
endif()

target_link_libraries(firmware PRIVATE
  ${FIRMWARE_LINK_LIBRARIES}
)

if(IS_STM32)
  add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
      $<TARGET_FILE:firmware>
      $<TARGET_FILE_DIR:firmware>/$<TARGET_FILE_BASE_NAME:firmware>.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary
      $<TARGET_FILE:firmware>
      $<TARGET_FILE_DIR:firmware>/$<TARGET_FILE_BASE_NAME:firmware>.bin
    COMMENT "firmware.hex and firmware.bin generated as well."
    VERBATIM
  )
endif()
