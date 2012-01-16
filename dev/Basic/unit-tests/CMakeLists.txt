#TODO: We can definitely automate this more.

#Add all unit tests in addition to all source files.
add_executable(SM_UnitTests ${TESTING_CPP} ${BASIC_CPP})

#Link this executable.
target_link_libraries (SM_UnitTests ${LibraryList})
target_link_libraries (SM_UnitTests cppunit)
