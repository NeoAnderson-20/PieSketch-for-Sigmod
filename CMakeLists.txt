cmake_minimum_required(VERSION 3.14)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O2 -g")
set(CMAKE_BUILD_TYPE "Debug")
add_compile_options(-mavx2)


include_directories(.)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")


# add_executable(ben_pkt ben_pkt.cc)
# # add_executable(ben_lat ben_lat.cc)
# add_executable(ben_seattle ben_seattle.cc)
# add_executable(ben_webget ben_webget.cc)
# add_executable(ben_docs ben_docs.cc)
# # 
add_executable(test test.cc)
# add_executable(simdtest simdtest.cc)