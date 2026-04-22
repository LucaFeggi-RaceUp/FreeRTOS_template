set(APP_SOURCES
  app.cpp
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
