#TODO: We can definitely automate this more.

MESSAGE("TEMP: ${BASIC_CPP}" )

#Add the buffering sub-folder's sources.
add_executable(SM_BufferedTests buffering/main.cpp buffering/BufferedUnitTests.cpp ${BASIC_CPP})

#Link this executable.
target_link_libraries (SM_BufferedTests ${LibraryList})
target_link_libraries (SM_BufferedTests cppunit)
