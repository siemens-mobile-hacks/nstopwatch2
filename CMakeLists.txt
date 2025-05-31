cmake_minimum_required(VERSION 3.28.2)

include(../sdk/config.cmake)
project(sieelf_nstopwatch2 C)

include_directories("../libsie/src/include")
link_libraries(crt crt_helper gcc c)
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(NStopWatch2_ELKA ${SOURCES})
target_sdk_setup(NStopWatch2_ELKA ELKA)
target_link_directories(NStopWatch2_ELKA PUBLIC "../libsie/build/lib/ELKA")
target_compile_options(NStopWatch2_ELKA BEFORE PUBLIC -std=c11 -Wall -Werror -DSWILIB_MODERN)
