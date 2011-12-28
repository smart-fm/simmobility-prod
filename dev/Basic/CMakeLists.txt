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

#Additional module find scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

#List of libraries to link
#SET(LibraryList soci_core-gcc-3_0 soci_postgresql-gcc-3_0)
SET(LibraryList "")

#Find boost
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0")
find_package(Boost COMPONENTS thread serialization date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
LIST(APPEND LibraryList ${Boost_LIBRARIES})

#Find TinyXML
find_package(TinyXML REQUIRED)
include_directories(${TINYXML_INCLUDE_DIR})
LIST(APPEND LibraryList ${TINYXML_LIBRARIES})

#Find PQ
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})
LIST(APPEND LibraryList ${PostgreSQL_LIBRARIES})

#Find SOCI
find_package(SOCI REQUIRED)
include_directories(${SOCI_INCLUDE_DIRS})
LIST(APPEND LibraryList ${SOCI_LIBRARIES})

#Find SOCI-PostgreSQL
find_package(SOCIPostgreSQL REQUIRED)
include_directories(${SOCIPOSTGRESQL_INCLUDE_DIRS})
LIST(APPEND LibraryList ${SOCIPOSTGRESQL_LIBRARIES})

#Optionally find: MPI, CGAL
IF (${SIMMOB_DISABLE_MPI} MATCHES "OFF")
  #Find boost-mpi
  find_package(Boost COMPONENTS mpi REQUIRED)
  LIST(APPEND LibraryList ${Boost_LIBRARIES})
  
  #Find any MPI implementation
  set(MPI_COMPILER ${MPI_ROOT}/bin/mpicxx)
  find_package(MPI REQUIRED)
  include_directories(${MPI_CXX_INCLUDE_PATH})
  LIST(APPEND LibraryList ${MPI_CXX_LIBRARIES})
  
  #More mpi-related flags
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
  
  #Find cgal
  find_package(CGAL REQUIRED)
  #include_directories(${CGAL_INCLUDE_DIR})  #This messes up gmpxx...
  LIST(APPEND LibraryList ${CGAL_LIBRARIES})
  
  #Use the mpi compiler.
  SET(CMAKE_C_COMPILER ${MPI_C_COMPILER})
  SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
  MESSAGE("INFO: Overriding c++ compiler: ${CMAKE_CXX_COMPILER}")
ENDIF(${SIMMOB_DISABLE_MPI} MATCHES "OFF")

#Remove duplicates, just in case
#TODO: We should probably build up a list of IncludeDirs and LibraryDirs too (duplicates removed)
#      Not a real priority right now, but should allow better porting later on.
list(REMOVE_DUPLICATES LibraryList)

#Group as many files as possible together automatically
FILE(GLOB_RECURSE BASIC_CPP *.cpp)

#Create a list of items to remove from the Build.
# Adding main.cpp to this list makes a good starting point, and ensures
# we always have at least one item to remove from BASIC_CPP
FILE(GLOB IgnoreList main.cpp)

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

#Include the top-level source/binary directories
include_directories("${PROJECT_SOURCE_DIR}")

#Build tests?
IF (${BUILD_TESTS} MATCHES "ON")
	add_subdirectory(unit-tests)
ENDIF (${BUILD_TESTS} MATCHES "ON")

#Our output executable and its input sources. List *.c files manually
add_executable(SimMobility main.cpp ${BASIC_CPP})

#Libraries to link, ported over from Eclipse.
target_link_libraries (SimMobility ${LibraryList})




