cmake_minimum_required(VERSION 3.25)
project(boost_chat)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}")

set(CMAKE_CXX_STANDARD 17)

add_executable(boost_chat main.cpp Server.h Server.cpp Session.h Session.cpp User.h User.cpp Message.h Message.cpp Room.h Room.cpp)
