#Required version of cmake
cmake_minimum_required (VERSION 2.8.8)

#Set our module override policy:
IF (POLICY CMP0017)
  cmake_policy(SET CMP0017 NEW)
ENDIF (POLICY CMP0017)

#Fairly reasonable way of determining if we're on OSX
#If you have an "/Applications" directory on *nix... just why?
IF(EXISTS "/Applications/")
  SET(USING_OSX ON)
ENDIF(EXISTS "/Applications/")

#Force a decent compiler on OSX (gcc is still 4.2 on OSX, and it lacks functionality we need) 
#This MUST go before the project() definition.
IF(USING_OSX)
  #Gcc 4.X?
  IF(EXISTS "/opt/local/bin/g++-mp-4.6")
    SET(CMAKE_C_COMPILER    "/opt/local/bin/gcc-mp-4.6")
    SET(CMAKE_CXX_COMPILER  "/opt/local/bin/g++-mp-4.6")
  ENDIF(EXISTS "/opt/local/bin/g++-mp-4.6")
  
  #Clang? (built from SVN, will override gcc)
  #IF(EXISTS "/usr/local/bin/clang")
  #  SET(CMAKE_C_COMPILER    "/usr/local/bin/clang")
  #  SET(CMAKE_CXX_COMPILER  "/usr/local/bin/clang++")
  #ENDIF(EXISTS "/usr/local/bin/clang")
ENDIF(USING_OSX)

#Project name. Used to tag resources in cmake. 
project (SimMob)

#Turn on verbose output.
SET(CMAKE_VERBOSE_MAKEFILE ON)

#Ensure that all executables get placed in the top-level build directory.
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Also: version number. This will go into the generated header file.
set (Basic_VERSION_MAJOR 0)
set (Basic_VERSION_MINOR 1)

#Flags for each configuration type
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

#Special case flag: required on OSX due to a (somewhat) glitchy libstdc++
#IF(USING_OSX)
#  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -D_GLIBCXX_FULLY_DYNAMIC_STRING=1")
#ENDIF(USING_OSX)

#TODO: We use fPIC for everything in Debug mode, to avoid building twice. 
#      This (may) make Sim Mobility slower in Debug mode. In release mode,
#      we build two versions of the code: non-PIC for the executable, and 
#      PIC for the library. Of course, this slows the build process down.
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -fPIC")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} -fPIC")
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

#Option: use new signal model. Use the cmake gui to change this on a per-user basis.
option(SIMMOB_NEW_SIGNAL "Use the new signal Model." ON)

#Option: Run SimMobility to only produce an input XML file for itself
option(SIMMOB_XML_WRITER "Sim Mobility will print the network as an XML file." OFF)

#Option: USE XML file as SimMobility input
option(SIMMOB_XML_READER "Load from XML, not the database" OFF)

#Option: Our new (new) XML classes. This option will disappear as soon as possible.
option(SIMMOB_PARTIAL_XML_READER "If XML_READER is on, use the slightly newer XML reader." OFF)

#Option: strict error handling. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_STRICT_AGENT_ERRORS  "All unexpected and potentially dangerous behavior causes Agents to throw exceptions. If OFF, Agents will generally just remove themselves from the simulation in case of error." OFF)

#Option: disable output. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_DISABLE_OUTPUT  "Disable all mutex-locked output and all additional non-trivial output. Avoid all use of mutexes w.r.t. output." OFF)

#Option: disable MPI. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_DISABLE_MPI  "Disable all included files, libraries, and code segments that require MPI." ON)

#Option: compile with c++0x. Use the cmake gui to change this on a per-user basis.
option (SIMMOB_LATEST_STANDARD  "Attempt to compile Sim Mobility with the latest version of C++ (2011). Untested." OFF)

#Option: generate parallel profile. Use the cmake gui to change this on a per-user basis.
option(SIMMOB_AGENT_UPDATE_PROFILE "Log all Agent update ticks per thread and start time. Useful for debugging parallelization errors, but very slow." OFF)

#Option: build tests. Use the cmake gui to change this on a per-user basis.
option(BUILD_TESTS "Build unit tests." OFF)

#Option: build short term. Use the cmake gui to change this on a per-user basis.
option(BUILD_SHORT "Build short-term simulator." ON)

#Option: build medium term. Use the cmake gui to change this on a per-user basis.
option(BUILD_MEDIUM "Build medium-term simulator." ON)

#Option: build long term. Use the cmake gui to change this on a per-user basis.
option(BUILD_LONG "Build long-term simulator." ON)    

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
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h.in"
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h"
)


#Cflags for C++0x support via gcc
IF (SIMMOB_LATEST_STANDARD)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ELSE (SIMMOB_LATEST_STANDARD)
  #Needed for profiling Agent updates (timing)
  #IF (${SIMMOB_AGENT_UPDATE_PROFILE} MATCHES "ON")
  #  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_XOPEN_SOURCE=700")
  #ENDIF (${SIMMOB_AGENT_UPDATE_PROFILE} MATCHES "ON")
ENDIF(SIMMOB_LATEST_STANDARD)



#Enable threading on GCC-4.7
IF(CMAKE_COMPILER_IS_GNUCXX)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  IF (GCC_VERSION VERSION_EQUAL 4.7)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_GLIBCXX_HAVE_GTHR_DEFAULT")
  ENDIF (GCC_VERSION VERSION_EQUAL 4.7)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


#Enable threading on Clang
#NOTE: This is only a problem if GCC 4.7 AND clang are installed. 
#      If you get errors about "DISABLE_THREADS" being set for boost, you can either enable the next
#      few lines, or compile against gcc-4.6's standard library. 
#IF(CMAKE_C_COMPILER MATCHES "/clang$" AND CMAKE_CXX_COMPILER MATCHES "/clang[+][+]$")
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_GLIBCXX_HAVE_GTHR_DEFAULT")
#ENDIF()


#Testing PGO: Have to pass these flags in from the command line.
#NOTE: PGO currently doesn't work right.
#SET(TestPgoDirectory "${CMAKE_CURRENT_SOURCE_DIR}/pgo")
#IF (SIMMOBTEST_GENERATE_PROFILE)
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate=${TestPgoDirectory}")
#  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-generate=${TestPgoDirectory}")
#  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate=${TestPgoDirectory}")
#ENDIF(SIMMOBTEST_GENERATE_PROFILE)
#IF (SIMMOBTEST_USE_PROFILE)
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use=${TestPgoDirectory}")
#  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-use=${TestPgoDirectory}")
#  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-use=${TestPgoDirectory}")
#ENDIF (SIMMOBTEST_USE_PROFILE)


#Avoid deprecation warnings in boost by declaring that this platform has no hash_set
# implementation (note: doesn't affect unordered_map; fixes a warning with gcc-4.5 and boost 1.42)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_NO_HASH")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#Additional module find scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

#MinGW helpers
include_directories("/mingw/include")

#List of libraries to link
SET(LibraryList "")

#Find boost
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0")
find_package(Boost COMPONENTS system thread serialization date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
LIST(APPEND LibraryList ${Boost_LIBRARIES})

#Find TinyXML
find_package(TinyXML REQUIRED)
include_directories(${TINYXML_INCLUDE_DIR})
LIST(APPEND LibraryList ${TINYXML_LIBRARIES})

#Find Xsd (headers only)
find_package(Xsd REQUIRED)
include_directories(${XSD_INCLUDE_DIR})

#Find XercesC
find_package(XercesC REQUIRED)
include_directories(${XERCESC_INCLUDE_DIR})
LIST(APPEND LibraryList ${XERCESC_LIBRARIES})

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

#Optionally find: MPI
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
  
  #Use the mpi compiler.
  SET(CMAKE_C_COMPILER ${MPI_C_COMPILER})
  SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
  MESSAGE("INFO: Overriding c++ compiler: ${CMAKE_CXX_COMPILER}")
ENDIF(${SIMMOB_DISABLE_MPI} MATCHES "OFF")


#Remove duplicates, just in case
#TODO: We should probably build up a list of IncludeDirs and LibraryDirs too (duplicates removed)
#      Not a real priority right now, but should allow better porting later on.
list(REMOVE_DUPLICATES LibraryList)

#Find all of our Shared source files
FILE(GLOB_RECURSE SharedCode_CPP shared/*.cpp)

#Remove tests
FILE(GLOB_RECURSE SharedCode_TEST "shared/unit-tests/*.cpp" "shared/unit-tests/*.c")
LIST(REMOVE_ITEM SharedCode_CPP ${SharedCode_TEST})

#All code has access to "shared/"
include_directories("${PROJECT_SOURCE_DIR}/shared")

#Build a shared object file for short-term, mid-term, long-term, and tests to use
add_library(SimMob_Shared OBJECT ${SharedCode_CPP})

#Build an *actual* shared object for the geospatial code & XML loader.
#TODO: For now we just include everything in the shared lib. Later we can slim it down.
IF (CMAKE_BUILD_TYPE MATCHES "Debug") 
  #The debug code already uses PIC
  add_library(simmob_geoxml SHARED $<TARGET_OBJECTS:SimMob_Shared>)
ELSEIF(CMAKE_BUILD_TYPE MATCHES "Release")
  #We need to re-build Release mode, since PIC is (likely) off.
  add_library(simmob_geoxml SHARED  ${SharedCode_CPP})
ENDIF()

#Build the Short term?
IF (${BUILD_SHORT} MATCHES "ON")
	add_subdirectory(short)
ENDIF (${BUILD_SHORT} MATCHES "ON")

#Build the Medium term?
IF (${BUILD_MEDIUM} MATCHES "ON")
	add_subdirectory(medium)
ENDIF (${BUILD_MEDIUM} MATCHES "ON")

#Build the Long term?
IF (${BUILD_LONG} MATCHES "ON")
	add_subdirectory(long)
ENDIF (${BUILD_LONG} MATCHES "ON")

#Build tests? 
IF (${BUILD_TESTS} MATCHES "ON")
	add_subdirectory(shared/unit-tests)
ENDIF (${BUILD_TESTS} MATCHES "ON")


# Based on http://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen/
# Add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doxy/SimMobility.doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

