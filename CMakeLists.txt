cmake_minimum_required(VERSION 2.6)

project(d-logg)

set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200112L -std=c99 -D_BSD_SOURCE")

add_executable(dlogg-reader dlogg-reader.c datatypes.c communication.c parsing.c logging.c)

install(TARGETS dlogg-reader RUNTIME DESTINATION bin)
