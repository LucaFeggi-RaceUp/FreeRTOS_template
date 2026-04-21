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

set(DRIVER_IDS_INCLUDE_DIR
  "${CMAKE_SOURCE_DIR}/include/driver_ids"
  CACHE PATH "Directory containing the public driver id headers"
)

if(NOT DEFINED DRIVERS_INSTANCE)
  set(DRIVERS_INSTANCE "${INSTANCE}")
endif()

set(DRIVERS_INSTANCE
  "${DRIVERS_INSTANCE}"
  CACHE STRING "Driver backend instance used by lib/drivers"
)

include(${INSTANCE_DIR}/CMakeLists.txt)


## Addition of targets -----

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE
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

add_subdirectory(src)

if(DEFINED FREERTOS_KERNEL_FORCE_INCLUDE)
  target_compile_options(src PRIVATE
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
    src
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
    src
    freertos_kernel
  )
endif()

target_link_libraries(firmware PRIVATE
  ${FIRMWARE_LINK_LIBRARIES}
)
