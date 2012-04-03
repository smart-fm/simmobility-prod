#Required version of cmake
cmake_minimum_required (VERSION 2.6)

#Project name. Used to tag resources in cmake. 
project (Traffic_Map_Decomposition_Eclipse)

#Flags for each configuration type
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

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


#Avoid deprecation warnings in boost by declaring that this platform has no hash_set
# implementation (note: doesn't affect unordered_map; fixes a warning with gcc-4.5 and boost 1.42)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_NO_HASH")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#List of libraries to link
SET(LibraryList "")

#Find boost
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0")
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
LIST(APPEND LibraryList ${Boost_LIBRARIES})

#Group as many files as possible together automatically
FILE(GLOB_RECURSE BASIC_CPP *.cpp)


#Remove the "Build" directory
FILE(GLOB_RECURSE IgnoreList "Build/*.c" "Build/*.cpp")

#Remove the "Debug" directories
FILE(GLOB_RECURSE IgnoreTmp "Debug/*.c" "Debug/*.cpp")
LIST(APPEND IgnoreList ${IgnoreTmp})
FILE(GLOB_RECURSE IgnoreTmp "Release/*.c" "Release/*.cpp")
LIST(APPEND IgnoreList ${IgnoreTmp})

#Actually remove these
LIST(REMOVE_ITEM BASIC_CPP ${IgnoreList})

#Include the top-level source/binary directories
include_directories("${PROJECT_SOURCE_DIR}")

#Our output executable and its input sources. List *.c files manually
add_executable(Traffic_Map_Decomposition_Eclipse ${BASIC_CPP})

#Libraries to link, ported over from Eclipse.
target_link_libraries (Traffic_Map_Decomposition_Eclipse ${LibraryList})


