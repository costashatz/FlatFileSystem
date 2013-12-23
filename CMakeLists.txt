cmake_minimum_required (VERSION 2.6)
project (FlatFileSystem)

set(ROOT .)

include_directories(${ROOT}/includes)

add_executable(Shell main.cpp)