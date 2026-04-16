message(STATUS "Configuring STM32 toolchain")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX arm-none-eabi)

if(NOT DEFINED MCPU)
  set(MCPU cortex-m33)
endif()

find_program(STM32_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc REQUIRED)
find_program(STM32_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++ REQUIRED)
find_program(STM32_ASM_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc REQUIRED)
find_program(STM32_OBJCOPY NAMES ${TOOLCHAIN_PREFIX}-objcopy REQUIRED)
find_program(STM32_SIZE NAMES ${TOOLCHAIN_PREFIX}-size REQUIRED)

set(CMAKE_C_COMPILER   "${STM32_C_COMPILER}" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${STM32_CXX_COMPILER}" CACHE FILEPATH "" FORCE)
set(CMAKE_ASM_COMPILER "${STM32_ASM_COMPILER}" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJCOPY      "${STM32_OBJCOPY}" CACHE FILEPATH "" FORCE)
set(CMAKE_SIZE         "${STM32_SIZE}" CACHE FILEPATH "" FORCE)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS S ASM s)
set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.s
  PROPERTIES LANGUAGE ASM
)
set(CMAKE_ASM_FLAGS
    "-mcpu=${MCPU} -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -x assembler-with-cpp"
)

set(CMAKE_C_FLAGS "-mcpu=${MCPU} -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "-mcpu=${MCPU} -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS "-mcpu=${MCPU} -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -x assembler-with-cpp")
# set(CMAKE_EXE_LINKER_FLAGS "-mcpu=${MCPU} -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -Wl,--gc-sections")


#add_compile_options(
#  -mcpu=${MCPU}
#  -mthumb
#  -mfloat-abi=hard
#  -mfpu=fpv5-sp-d16
#  -ffunction-sections
#  -fdata-sections
#)

if(DEFINED LINKER_SCRIPT_FOLDER AND DEFINED STM32_DEVICE)
  add_link_options(
    -mcpu=${MCPU}
    -mthumb
    -mfloat-abi=hard
    -mfpu=fpv5-sp-d16
    -Wl,--gc-sections
    -T${LINKER_SCRIPT_FOLDER}${STM32_DEVICE}_FLASH.ld
  )
endif()
