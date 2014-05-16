#Include the "long" directory  
include_directories("${PROJECT_SOURCE_DIR}/long")

#Additional module find scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

#Find CPP Unit
find_package(CppUnit REQUIRED)
include_directories(${CPPUNIT_INCLUDE_DIR})
LIST(APPEND UnitTestLibs ${CPPUNIT_LIBRARY})

#Find all source files in unit test
FILE(GLOB_RECURSE LongTerm_TEST "main.cpp" "unit-tests/*.cpp""unit-tests/*.hpp")

#Find all cpp files in this directory
FILE(GLOB_RECURSE LongTerm_CPP ../*.cpp)

#Remove the main file in long dir
FILE(GLOB_RECURSE LongTermMain_CPP ../main.cpp)
LIST(REMOVE_ITEM LongTerm_CPP ${LongTermMain_CPP} ${LongTerm_TEST})

#create SimMob_Long shared object that include source files for testing
add_library(SimMob_Long OBJECT ${LongTerm_CPP})

#Add all unit tests in addition to all source files.
add_executable(SimMobility_Long_test ${LongTerm_TEST} $<TARGET_OBJECTS:SimMob_Shared> $<TARGET_OBJECTS:SimMob_Long>)

#Link this executable.
target_link_libraries (SimMobility_Long_test ${LibraryList} ${UnitTestLibs})

