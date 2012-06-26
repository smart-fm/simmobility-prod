#Include the "short" directory  
include_directories("${PROJECT_SOURCE_DIR}/short")

#If "BUILD_TESTS" is set, we should add the unit-tests directory.
# TODO: Currently no short-term unit tests 

#Find all cpp files in this directory
FILE(GLOB_RECURSE ShortTerm_CPP *.cpp)

#Remove the unit tests
# TODO: Currently no short-term unit tests
#FILE(GLOB_RECURSE ShortTerm_TEST "unit-tests/*.cpp" "unit-tests/*.c")
#LIST(REMOVE_ITEM ShortTerm_CPP ${ShortTerm_TEST})

#Create the short-term simulator
add_executable(SimMobility_Short ${ShortTerm_CPP} $<TARGET_OBJECTS:SimMob_Shared>)
 
#Link this executable.
target_link_libraries (SimMobility_Short ${LibraryList})

#Link it to the timer class if we're generating a profiling build.
IF (${SIMMOB_AGENT_UPDATE_PROFILE} MATCHES "ON")
  target_link_libraries (SimMobility_Short "rt")
ENDIF (${SIMMOB_AGENT_UPDATE_PROFILE} MATCHES "ON")