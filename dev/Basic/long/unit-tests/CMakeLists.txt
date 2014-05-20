#Re-generating this is necessary to get the latest define ("SIMMOB_USE_TEST_GUI").  
#It appears to be harmless... perhaps there's a better way to do it?
configure_file (
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h.in"
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h"
)

#Include the "unit-tests" directory  
include_directories("unit-tests")

#Find all source files in unit test
FILE(GLOB_RECURSE LongTerm_TEST "*.cpp" "*.hpp")

#Add all unit tests in addition to all source files.
add_executable(SM_UnitTests_Long ${LongTerm_TEST} $<TARGET_OBJECTS:SimMob_Shared> $<TARGET_OBJECTS:SimMob_Long>)

#Link this executable.
target_link_libraries (SM_UnitTests_Long ${LibraryList} ${UnitTestLibs})

