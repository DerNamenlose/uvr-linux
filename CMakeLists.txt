cmake_minimum_required(VERSION 2.6)

project(d-logg)

add_executable(dlogg-reader dlogg-reader.c datatypes.c communication.c parsing.c)

install(TARGETS dlogg-reader RUNTIME DESTINATION bin)
