# Toolchain file for GNU Arm Embedded (arm-none-eabi-gcc).
# CPU flags are added per target by the family file (they depend on the MCU/core).
# Based on umcub cmake/arm-none-eabi.cmake.

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED AMBI_TOOLCHAIN_PREFIX)
  set(AMBI_TOOLCHAIN_PREFIX arm-none-eabi-)
endif()

set(CMAKE_C_COMPILER   ${AMBI_TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${AMBI_TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${AMBI_TOOLCHAIN_PREFIX}objcopy CACHE FILEPATH "")
set(CMAKE_SIZE         ${AMBI_TOOLCHAIN_PREFIX}size CACHE FILEPATH "")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")

set(CMAKE_C_FLAGS_INIT   "-ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS_INIT "-x assembler-with-cpp")
set(CMAKE_C_FLAGS_DEBUG          "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE        "-O3 -g")
