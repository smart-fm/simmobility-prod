#Required version of cmake
cmake_minimum_required (VERSION 2.8)

#Project name. Used to tag resources in cmake
project (Basic)

#Turn on verbose output
SET(CMAKE_VERBOSE_MAKEFILE ON)

#Force gcc to output single line errors. 
# This makes it easier for Eclipse to parse and understand each error.
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


#Include directories ported over from Eclipse. Can be augmented later with pkgconfig.
include_directories("/usr/include/soci/postgresql")
include_directories("/usr/include/soci")
include_directories("/usr/include/postgresql")

#Group as many files as possible together automatically
FILE(GLOB_RECURSE BASIC_CPP RECURSE_SYMLINKS_OFF *.cpp)

#Remove tests
FILE(GLOB UnitTests buffering/unit-tests/*.cpp)
LIST(REMOVE_ITEM BASIC_CPP ${UnitTests})

#Include the top-level source/binary directories
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")

#Our output executable and its input sources. List *.c files manually
add_executable(SimMobility main.cpp  ${BASIC_CPP})
