# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version

cmake_minimum_required(VERSION 3.16)
project(
  snakegame
  LANGUAGES CXX
  DESCRIPTION NCurses_Project
  VERSION 0.1)

message(STATUS ">> PROJECT_NAME: ${PROJECT_NAME}")
find_package(Curses REQUIRED)
find_package(Threads)

# print out the properties of dev environment of ncurses
message(STATUS ">> CURSES_INCLUDE_DIRS: ${CURSES_INCLUDE_DIRS}")
message(STATUS ">> CURSES_LIBRARY: ${CURSES_LIBRARIES}")

# tell the build to include the headers from ncurses
include_directories(${CURSES_INCLUDE_DIRS})

#specify the name of the executable to be built
add_executable(${PROJECT_NAME}
  src/main.cpp
  src/timer.h)

# tell it to link the executable target against ncurses
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(${PROJECT_NAME}
  PUBLIC
  include)
