cmake_minimum_required(VERSION 2.8)

#Set our module override policy:
IF (POLICY CMP0017)
  cmake_policy(SET CMP0017 NEW)
ENDIF (POLICY CMP0017)

#Project name. Used to tag resources in cmake. 
project (relay)

#Turn on verbose output.
SET(CMAKE_VERBOSE_MAKEFILE ON)

#Ensure that all executables get placed in the top-level build directory.
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#Flags for each configuration type
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")


#Force gcc to output single line errors. 
# This makes it easier for Eclipse to parse and understand each error.
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#Additional module find scripts
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

#List of libraries to link
SET(LibraryList "")

#Find boost
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0" "1.53" "1.53.0" "1.54" "1.54.0")
find_package(Boost COMPONENTS system thread filesystem serialization date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
LIST(APPEND LibraryList ${Boost_LIBRARIES})

#Find all source files.
#FILE(GLOB_RECURSE SourceCpp src/*.cpp)
#FILE(GLOB_RECURSE SourceHpp src/*.hpp)

#All code has access to "shared/"
include_directories("${PROJECT_SOURCE_DIR}/src")


#Build it.
add_executable(relay "main.cpp")
 
#Link this executable.
target_link_libraries (relay ${LibraryList})

