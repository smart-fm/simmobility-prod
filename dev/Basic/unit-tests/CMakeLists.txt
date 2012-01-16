#TODO: We can definitely automate this more.

#Additional include directory for QT 
#NOTE: This is experimental; will use "find" later if it's useful.
include_directories("/usr/include/qt4")

#Add all unit tests in addition to all source files.
add_executable(SM_UnitTests ${TESTING_CPP} ${BASIC_CPP})

#Link this executable.
target_link_libraries (SM_UnitTests ${LibraryList})
target_link_libraries (SM_UnitTests cppunit)

#Additional link library; QT framework
#NOTE: This is experimental; will use "find" later if it's useful.
target_link_libraries (SM_UnitTests qxcppunitd)
