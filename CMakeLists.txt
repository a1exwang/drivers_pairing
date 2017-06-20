CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
PROJECT(drivers_pairing)
ADD_EXECUTABLE(six_six_six main.cpp pairing.cpp km.cpp)
ADD_EXECUTABLE(km my_km.cpp km.cpp)
