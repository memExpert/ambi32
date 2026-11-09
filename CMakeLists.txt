cmake_minimum_required(VERSION 3.20)
project(ambi C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(AMBI_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(AMBI_BOARD "blackpill_f411ce" CACHE STRING "Board directory under boards/")
set(AMBI_BOARD_DIR ${AMBI_ROOT}/boards/${AMBI_BOARD})
if(NOT EXISTS ${AMBI_BOARD_DIR}/ambi_board.h)
  message(FATAL_ERROR "ambi: no board '${AMBI_BOARD}' (boards/${AMBI_BOARD}/ambi_board.h)")
endif()
include(${AMBI_BOARD_DIR}/board.cmake)
include(${AMBI_ROOT}/cmake/families/${AMBI_FAMILY}.cmake)

# Debug log over the board's UART in Debug builds only.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(AMBI_LOG 1)
else()
  set(AMBI_LOG 0)
endif()

set(AMBI_SOURCES
  ${AMBI_ROOT}/src/app/ambi_main.c
  ${AMBI_FAMILY_SOURCES}
)
if(AMBI_LOG)
  list(APPEND AMBI_SOURCES
    ${AMBI_ROOT}/src/util/ambi_log.c
    ${AMBI_ROOT}/src/util/ambi_ring.c
    ${AMBI_ROOT}/port/arch/cortexm/newlib_syscalls.c
    ${AMBI_FAMILY_LOG_SOURCES})
endif()

add_executable(ambi ${AMBI_SOURCES})
target_include_directories(ambi PRIVATE
  ${AMBI_BOARD_DIR}
  ${AMBI_ROOT}/port/include
  ${AMBI_ROOT}/src/util/include
  ${AMBI_FAMILY_INCLUDES}
)
target_compile_definitions(ambi PRIVATE ${AMBI_FAMILY_DEFS} AMBI_LOG=${AMBI_LOG})
target_compile_options(ambi PRIVATE ${AMBI_CPU_FLAGS} $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -Werror>)

# Linker script: the family template preprocessed with the board header.
set(_ld ${CMAKE_BINARY_DIR}/ambi_app.ld)
add_custom_command(OUTPUT ${_ld}
  COMMAND ${CMAKE_C_COMPILER} -E -P -x c -DAMBI_LOG=${AMBI_LOG} -I${AMBI_BOARD_DIR} -I${AMBI_ROOT}/port/include
          ${AMBI_FAMILY_LD_IN} -o ${_ld}
  DEPENDS ${AMBI_FAMILY_LD_IN} ${AMBI_BOARD_DIR}/ambi_board.h ${AMBI_ROOT}/port/include/ambi_pin.h
  COMMENT "Linker script for ${AMBI_BOARD}")
add_custom_target(ambi_ld DEPENDS ${_ld})
add_dependencies(ambi ambi_ld)
set_target_properties(ambi PROPERTIES LINK_DEPENDS "${_ld};${AMBI_ROOT}/port/arch/cortexm/cortexm_sections.ld.inc")
target_link_options(ambi PRIVATE ${AMBI_CPU_FLAGS}
  -T${_ld} -L${AMBI_ROOT}/port/arch/cortexm
  -nostartfiles --specs=nano.specs
  -Wl,--gc-sections -Wl,-Map=${CMAKE_BINARY_DIR}/ambi.map -Wl,--print-memory-usage)

add_custom_command(TARGET ambi POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:ambi> ${CMAKE_BINARY_DIR}/ambi.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:ambi>
  VERBATIM)
