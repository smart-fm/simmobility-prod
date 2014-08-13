#Re-generating this is necessary to get the latest define ("SIMMOB_USE_TEST_GUI").  
#It appears to be harmless... perhaps there's a better way to do it?
configure_file (
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h.in"
  "${PROJECT_SOURCE_DIR}/shared/GenConfig.h"
)


#Add all unit tests in addition to all source files.
add_executable(SM_UnitTests ${SharedCode_TEST} $<TARGET_OBJECTS:SimMob_Shared>)
 
#Link this executable.
target_link_libraries (SM_UnitTests ${LibraryList}  ${UnitTestLibs})

