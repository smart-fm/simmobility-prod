#Required version of cmake
cmake_minimum_required (VERSION 2.8)

#Project name. Used to tag resources in cmake. 
project (Basic)

# Also: version number. This will go into the generated header file.
set (Basic_VERSION_MAJOR 0)
set (Basic_VERSION_MINOR 1)

#Option: disable MPI. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_DISABLE_MPI  "Disable all included files, libraries, and code segments that require MPI." ON)

#Option: disable dynamic dispatch. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_DISABLE_DYNAMIC_DISPATCH  "Disable dynamic dispatch of Agents. All Agents will be started at time tick zero and will silently skup updates until before their startTime (or after they are done)." OFF)

#Option: compile with c++0x. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_LATEST_STANDARD  "Attempt to compile Sim Mobility with the latest version of C++ (2011). Untested." OFF)

#Option: build tests. Use the cmake gui to change this on a per-user basis.
option(BUILD_TESTS "Build unit tests." OFF) 

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

#Generate a header file to contain configuration settings. This is useful
#  for, e.g., cmake variable which can then be set via the gui. 
configure_file (
  "${PROJECT_SOURCE_DIR}/GenConfig.h.in"
  "${PROJECT_SOURCE_DIR}/GenConfig.h"
)

#Cflags for C++0x support via gcc
IF (SIMMOB_LATEST_STANDARD)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(SIMMOB_LATEST_STANDARD)


#Include directories ported over from Eclipse. Can be augmented later with pkgconfig.
include_directories("/usr/include/soci/postgresql")
include_directories("/usr/include/soci")
include_directories("/usr/include/postgresql")

#Optional includes: MPI
IF (${SIMMOB_DISABLE_MPI} MATCHES "OFF")
  include_directories("/usr/include/mpi")
ENDIF(${SIMMOB_DISABLE_MPI} MATCHES "OFF")

#Link directories
#link_directories (/usr/lib/whatever)

#Group as many files as possible together automatically
FILE(GLOB_RECURSE BASIC_CPP *.cpp)

#Create a list of items to remove from the Build.
# Adding main.cpp to this list makes a good starting point, and ensures
# we always have at least one item to remove from BASIC_CPP
#SET(IgnoreList main.cpp)

#TEMP: Why isn't the previous line working?
FILE(GLOB IgnoreList "main.cpp")

#Remove tests
FILE(GLOB_RECURSE IgnoreTmp "unit-tests/*.cpp" "unit-tests/*.c")
LIST(APPEND IgnoreList ${IgnoreTmp})

#Remove the "Build" directory
FILE(GLOB_RECURSE IgnoreTmp "Build/*.c" "Build/*.cpp")
LIST(APPEND IgnoreList ${IgnoreTmp})

#Remove the "Debug" directories
FILE(GLOB_RECURSE IgnoreTmp "Debug/*.c" "Debug/*.cpp")
LIST(APPEND IgnoreList ${IgnoreTmp})
FILE(GLOB_RECURSE IgnoreTmp "Debug_M/*.c" "Debug_M/*.cpp")
LIST(APPEND IgnoreList ${IgnoreTmp})
FILE(GLOB_RECURSE IgnoreTmp "Release/*.c" "Release/*.cpp")
LIST(APPEND IgnoreList ${IgnoreTmp})

#Actually remove these
LIST(REMOVE_ITEM BASIC_CPP ${IgnoreList})

#List of libraries to link
SET(LibraryList boost_thread boost_serialization tinyxml soci_core-gcc-3_0 soci_postgresql-gcc-3_0)

#Optional library: MPI. Also pulls in CGAL
IF (${SIMMOB_DISABLE_MPI} MATCHES "OFF")
  LIST(APPEND LibraryList boost_mpi CGAL)
ENDIF(${SIMMOB_DISABLE_MPI} MATCHES "OFF")

#Include the top-level source/binary directories
include_directories("${PROJECT_SOURCE_DIR}")
#include_directories("${PROJECT_BINARY_DIR}")

#Build tests?
IF (${BUILD_TESTS} MATCHES "ON")
	add_subdirectory(unit-tests)
ENDIF (${BUILD_TESTS} MATCHES "ON")

#Our output executable and its input sources. List *.c files manually
add_executable(SimMobility main.cpp ${BASIC_CPP})

#Libraries to link, ported over from Eclipse.
target_link_libraries (SimMobility ${LibraryList})




