set(_drivers_instance_dir "${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/../instance_layout.cmake")

find_package(Threads REQUIRED)

set(POSIX_SERIAL_DEVICE "/dev/tty_test" CACHE STRING
  "Device path used by the posix Serial driver"
)

if(DEFINED POSIX_FLASH_FILE)
  set(_posix_nv_memory_file_default "${POSIX_FLASH_FILE}")
else()
  set(_posix_nv_memory_file_default "${CMAKE_BINARY_DIR}/posix_nv_memory.bin")
endif()

set(POSIX_NV_MEMORY_FILE "${_posix_nv_memory_file_default}" CACHE FILEPATH
  "Backing file used by the posix NvMemory driver"
)

set(POSIX_GENERATED_SOCKET "/tmp/virtual_driver_test.sock" CACHE STRING
  "UNIX socket path used by the generated posix virtual driver API"
)

list(APPEND DRIVERS_INSTANCE_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/utils/test/virtual_driver_test.cpp"
)

list(APPEND DRIVERS_INSTANCE_PRIVATE_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/utils/test"
)

list(APPEND DRIVERS_INSTANCE_COMPILE_DEFINITIONS
  RU_POSIX_SERIAL_DEVICE=\"${POSIX_SERIAL_DEVICE}\"
  RU_POSIX_NV_MEMORY_FILE=\"${POSIX_NV_MEMORY_FILE}\"
  RU_POSIX_GENERATED_SOCKET=\"${POSIX_GENERATED_SOCKET}\"
)

list(APPEND DRIVERS_INSTANCE_PUBLIC_LINK_LIBRARIES
  Threads::Threads
)
