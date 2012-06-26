#Include the "medium" directory  
include_directories("${PROJECT_SOURCE_DIR}/medium")

#If "BUILD_TESTS" is set, we should add the unit-tests directory.
# TODO: Currently no medium-term unit tests 

#Find all cpp files in this directory
FILE(GLOB_RECURSE MediumTerm_CPP *.cpp)

#Remove the unit tests
# TODO: Currently no medium-term unit tests
#FILE(GLOB_RECURSE MediumTerm_TEST "unit-tests/*.cpp" "unit-tests/*.c")
#LIST(REMOVE_ITEM MediumTerm_CPP ${MediumTerm_TEST})

#Create the medium-term simulator
add_executable(SimMobility_Medium ${MediumTerm_CPP} $<TARGET_OBJECTS:SimMob_Shared>)
 
#Link this executable.
target_link_libraries (SimMobility_Medium ${LibraryList})
