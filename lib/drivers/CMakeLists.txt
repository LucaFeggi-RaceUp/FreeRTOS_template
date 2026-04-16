# Driver library local configuration.
#
# Required variables:
#   - DRIVER_IDS_INCLUDE_DIR: directory containing adc_id.hpp, gpio_id.hpp, ...
#
# Optional variables:
#   - DRIVERS_INSTANCE: explicit driver backend to use
#   - INSTANCE: fallback project instance name; mapped when possible

set(_drivers_supported_instances
  host
  stm32h5xx
)

if(DEFINED DRIVERS_INSTANCE)
  set(_drivers_requested_instance "${DRIVERS_INSTANCE}")
elseif(DEFINED INSTANCE)
  set(_drivers_requested_instance "${INSTANCE}")
else()
  message(FATAL_ERROR
    "No driver instance selected. "
    "Pass -DDRIVERS_INSTANCE=<host|stm32h5xx> "
    "or define INSTANCE in the parent project."
  )
endif()

set(_drivers_instance "${_drivers_requested_instance}")

# Map project-level instance names to the driver backend directory when possible.
if(_drivers_instance MATCHES "^stm32h5")
  set(_drivers_instance "stm32h5xx")
endif()

list(FIND _drivers_supported_instances "${_drivers_instance}" _drivers_instance_index)
if(_drivers_instance_index LESS 0)
  message(FATAL_ERROR
    "Unsupported driver instance '${_drivers_requested_instance}'. "
    "Supported values: ${_drivers_supported_instances}."
  )
endif()

if(NOT DEFINED DRIVER_IDS_INCLUDE_DIR)
  message(FATAL_ERROR
    "DRIVER_IDS_INCLUDE_DIR is required. "
    "Pass the directory that contains adc_id.hpp, can_id.hpp, ..."
  )
endif()

if(NOT IS_DIRECTORY "${DRIVER_IDS_INCLUDE_DIR}")
  message(FATAL_ERROR
    "DRIVER_IDS_INCLUDE_DIR='${DRIVER_IDS_INCLUDE_DIR}' is not a valid directory."
  )
endif()

set(_drivers_root_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(_drivers_instance_dir "${_drivers_root_dir}/instances/${_drivers_instance}")
set(_drivers_instance_cmake "${_drivers_instance_dir}/CMakeLists.txt")

if(NOT EXISTS "${_drivers_instance_cmake}")
  message(FATAL_ERROR
    "drivers: backend '${_drivers_instance}' is missing ${_drivers_instance_cmake}"
  )
endif()

set(DRIVERS_INSTANCE_SOURCES)
set(DRIVERS_INSTANCE_PUBLIC_HEADERS)
set(DRIVERS_INSTANCE_OPAQUE_HEADERS)
set(DRIVERS_INSTANCE_PUBLIC_INCLUDE_DIRS)
set(DRIVERS_INSTANCE_PRIVATE_INCLUDE_DIRS)
set(DRIVERS_INSTANCE_COMPILE_DEFINITIONS)
set(DRIVERS_INSTANCE_COMPILE_OPTIONS)
set(DRIVERS_INSTANCE_PUBLIC_LINK_LIBRARIES)
set(DRIVERS_INSTANCE_PRIVATE_LINK_LIBRARIES)

include("${_drivers_instance_cmake}")

set(_drivers_expected_target)

if(TARGET tl::expected)
  set(_drivers_expected_target tl::expected)
else()
  find_package(tl-expected CONFIG QUIET)

  if(TARGET tl::expected)
    set(_drivers_expected_target tl::expected)
  else()
    find_path(DRIVERS_TL_EXPECTED_INCLUDE_DIR
      NAMES tl/expected.hpp
      HINTS
        "${CMAKE_SOURCE_DIR}/third_party/expected/include"
        "${PROJECT_SOURCE_DIR}/third_party/expected/include"
      DOC "Directory containing tl/expected.hpp"
    )

    if(NOT DRIVERS_TL_EXPECTED_INCLUDE_DIR)
      message(FATAL_ERROR
        "drivers: tl/expected.hpp not found. "
        "Install tl-expected or pass -DDRIVERS_TL_EXPECTED_INCLUDE_DIR=/path/to/include."
      )
    endif()

    add_library(drivers_tl_expected INTERFACE)
    target_include_directories(drivers_tl_expected SYSTEM INTERFACE
      "${DRIVERS_TL_EXPECTED_INCLUDE_DIR}"
    )
    set(_drivers_expected_target drivers_tl_expected)
  endif()
endif()

set(_drivers_needs_c FALSE)
foreach(_drivers_source IN LISTS DRIVERS_INSTANCE_SOURCES)
  get_filename_component(_drivers_source_ext "${_drivers_source}" EXT)
  if(_drivers_source_ext STREQUAL ".c")
    set(_drivers_needs_c TRUE)
    break()
  endif()
endforeach()

if(_drivers_needs_c)
  enable_language(C)
endif()

add_library(drivers STATIC)

target_sources(drivers
  PRIVATE
    ${DRIVERS_INSTANCE_SOURCES}
  PUBLIC
    ${DRIVERS_INSTANCE_PUBLIC_HEADERS}
    ${DRIVERS_INSTANCE_OPAQUE_HEADERS}
)

target_include_directories(drivers
  PUBLIC
    ${DRIVERS_INSTANCE_PUBLIC_INCLUDE_DIRS}
  PRIVATE
    ${DRIVERS_INSTANCE_PRIVATE_INCLUDE_DIRS}
)

target_compile_definitions(drivers
  PRIVATE
    ${DRIVERS_INSTANCE_COMPILE_DEFINITIONS}
)

target_compile_options(drivers
  PRIVATE
    ${DRIVERS_INSTANCE_COMPILE_OPTIONS}
)

target_compile_features(drivers PUBLIC cxx_std_17)

target_link_libraries(drivers
  PUBLIC
    ${_drivers_expected_target}
    ${DRIVERS_INSTANCE_PUBLIC_LINK_LIBRARIES}
  PRIVATE
    ${DRIVERS_INSTANCE_PRIVATE_LINK_LIBRARIES}
)

list(LENGTH DRIVERS_INSTANCE_SOURCES _drivers_sources_count)

message(STATUS
  "drivers: using backend '${_drivers_instance}' "
  "(requested '${_drivers_requested_instance}') "
  "with ${_drivers_sources_count} source file(s)"
)
