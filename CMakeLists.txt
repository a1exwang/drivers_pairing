CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

#ADD_SUBDIRECTORY(munkres-cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

PROJECT(drivers_pairing)
ADD_EXECUTABLE(six_six_six main.cpp pairing.cpp)
#target_link_libraries (six_six_six munkres)
#include_directories (${PROJECT_SOURCE_DIR}/munkres-cpp/src)
