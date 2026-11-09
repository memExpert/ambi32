# Host (x86) tests of the hardware-free code: src/util, src/pixel, src/proto.
#   cmake -S tests/host -B build/host && cmake --build build/host && ctest --test-dir build/host
cmake_minimum_required(VERSION 3.20)
project(ambi_host_tests C)
enable_testing()

set(AMBI_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

option(AMBI_HOST_SANITIZE "Build host tests with ASan/UBSan (needs libasan/libubsan)" OFF)
if(AMBI_HOST_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR} ${AMBI_ROOT}/src/util/include ${AMBI_ROOT}/port/include)

function(ambi_host_test name)
  add_executable(test_${name} test_${name}.c ${ARGN})
  add_test(NAME ${name} COMMAND test_${name})
endfunction()

ambi_host_test(ring ${AMBI_ROOT}/src/util/ambi_ring.c)
ambi_host_test(log ${AMBI_ROOT}/src/util/ambi_log.c ${AMBI_ROOT}/src/util/ambi_ring.c)
target_compile_definitions(test_log PRIVATE AMBI_LOG=1)
