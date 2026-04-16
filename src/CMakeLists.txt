set(SRC_SOURCES
  app.cpp
)

add_library(src STATIC
  ${SRC_SOURCES}
)

target_include_directories(src PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(src PUBLIC
  freertos_kernel
  drivers
)
