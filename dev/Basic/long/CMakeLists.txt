#Include the "long" directory  
include_directories("${PROJECT_SOURCE_DIR}/long")

#If "BUILD_TESTS" is set, we should add the unit-tests directory.
# TODO: Currently no long-term unit tests 

#Find all cpp files in this directory
FILE(GLOB_RECURSE LongTerm_CPP *.cpp)

#Remove the unit tests
FILE(GLOB_RECURSE LongTerm_TEST "unit-tests/*.cpp" "unit-tests/*.c")
LIST(REMOVE_ITEM LongTerm_CPP ${LongTerm_TEST})

#Create the long-term simulator
add_executable(SimMobility_Long ${LongTerm_CPP} $<TARGET_OBJECTS:SimMob_Shared>)
 
#Link this executable.
target_link_libraries (SimMobility_Long ${LibraryList})

#Build into a library if requested
IF (${BUILD_LIBS} MATCHES "ON")
  add_library(simmob_long SHARED  ${LongTerm_CPP})
  install(DIRECTORY ./ DESTINATION include/sim_mob_long FILES_MATCHING PATTERN "*.hpp")
  INSTALL(TARGETS simmob_long RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ENDIF()
