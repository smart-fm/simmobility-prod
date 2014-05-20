#Include the "unit-tests" directory  
include_directories("unit-tests")

#Find all source files in unit test
FILE(GLOB_RECURSE LongTerm_TEST "main.cpp" "unit-tests/*.cpp" "unit-tests/*.hpp")

#Add all unit tests in addition to all source files.
add_executable(SM_UnitTests_Long ${LongTerm_TEST} $<TARGET_OBJECTS:SimMob_Shared> $<TARGET_OBJECTS:SimMob_Long>)

#Link this executable.
target_link_libraries (SM_UnitTests_Long ${LibraryList} ${UnitTestLibs})

