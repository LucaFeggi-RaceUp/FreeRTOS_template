file(GLOB DRIVERS_INSTANCE_SOURCES
  "${_drivers_instance_dir}/*.cpp"
  "${_drivers_instance_dir}/opaque/*.cpp"
  "${_drivers_instance_dir}/utils/*.cpp"
)
list(SORT DRIVERS_INSTANCE_SOURCES)

file(GLOB DRIVERS_INSTANCE_PUBLIC_HEADERS
  "${_drivers_root_dir}/include/*.hpp"
)
list(SORT DRIVERS_INSTANCE_PUBLIC_HEADERS)

file(GLOB DRIVERS_INSTANCE_OPAQUE_HEADERS
  "${_drivers_instance_dir}/opaque/*.hpp"
)
list(SORT DRIVERS_INSTANCE_OPAQUE_HEADERS)

set(DRIVERS_INSTANCE_PUBLIC_INCLUDE_DIRS
  "${_drivers_root_dir}/include"
  "${DRIVER_IDS_INCLUDE_DIR}"
  "${_drivers_instance_dir}/opaque"
)

set(DRIVERS_INSTANCE_PRIVATE_INCLUDE_DIRS
  "${_drivers_instance_dir}"
)
