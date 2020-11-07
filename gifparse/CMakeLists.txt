cmake_minimum_required(VERSION 3.10)

# set the project name
project(blarg_tools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package( Threads )

# add the executable
add_executable(gif_extract gif_extract.cpp gif_parser.hpp gifdec.c)
add_executable(frame_build frame_build.cpp gif_parser.hpp gifdec.c)

target_link_libraries(gif_extract ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(gif_extract stdc++fs)