#Include the "long" directory  
include_directories("${PROJECT_SOURCE_DIR}/long") 

#Find all cpp files in this directory
FILE(GLOB_RECURSE LongTerm_CPP *.cpp)

#Remove the unit tests
FILE(GLOB_RECURSE LongTerm_TEST "unit-tests/*.cpp" "unit-tests/*.hpp")
LIST(REMOVE_ITEM LongTerm_CPP ${LongTerm_TEST})

#Remove main.
FILE(GLOB_RECURSE LongTerm_TEST "main.cpp")
LIST(REMOVE_ITEM LongTerm_CPP ${LongTerm_TEST})

#Build a cmake shared object.
add_library(SimMob_Long OBJECT ${LongTerm_CPP})

#Create the long-term simulator
add_executable(SimMobility_Long "main.cpp" $<TARGET_OBJECTS:SimMob_Shared> $<TARGET_OBJECTS:SimMob_Long>)
 
#Link this executable.
target_link_libraries (SimMobility_Long ${LibraryList})

#Build tests for long term? 
IF (${BUILD_TESTS_LONG} MATCHES "ON")
	add_subdirectory(unit-tests)
ENDIF ()
