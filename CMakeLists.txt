project(FakkuDownloader CXX)

cmake_minimum_required(VERSION 2.8.9)

find_package(Qt5 COMPONENTS Core Network)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# C++11
if (CMAKE_COMPILER_IS_GNUCXX)
   execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   if (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
        message(STATUS "C++11 activated.")
        add_definitions("-std=gnu++11")
   elseif (GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
        message(WARNING "C++0x activated. If you get any errors update to a compiler which fully supports C++11")
        add_definitions("-std=gnu++0x")
   else ()
        message(FATAL_ERROR "C++11 needed. Therefore a gcc compiler with a version higher than 4.3 is needed.")   
   endif()
else (CMAKE_COMPILER_IS_GNUCXX)
   add_definitions("-std=c++0x")
endif (CMAKE_COMPILER_IS_GNUCXX)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# src
add_subdirectory(src)


