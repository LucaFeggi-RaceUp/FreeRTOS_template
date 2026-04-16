set(_drivers_instance_dir "${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/../instance_layout.cmake")

if(WIN32)
  list(APPEND DRIVERS_INSTANCE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/platform/win32.cpp"
  )
elseif(UNIX)
  list(APPEND DRIVERS_INSTANCE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/platform/posix.cpp"
  )
endif()
