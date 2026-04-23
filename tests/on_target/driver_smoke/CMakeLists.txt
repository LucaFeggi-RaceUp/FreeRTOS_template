set(APP_SOURCES
  app.cpp
  test_adc.cpp
  test_can.cpp
  test_nv_memory.cpp
  test_freertos.cpp
  test_report.cpp
)

add_library(app STATIC
  ${APP_SOURCES}
)

target_include_directories(app PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(app PUBLIC
  freertos_kernel
  drivers
)
