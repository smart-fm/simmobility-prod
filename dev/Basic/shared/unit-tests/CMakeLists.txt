#TODO: We can definitely automate this more.

#Additional include directory for QT 
#NOTE: This is experimental; will use "find" later if it's useful.
include_directories("/usr/include/qt4")

#Find CPP Unit
find_package(CppUnit REQUIRED)
include_directories(${CPPUNIT_INCLUDE_DIR})

#Find QxCppUnit
find_package(QxCppUnit REQUIRED)
include_directories(${QXCPPUNIT_INCLUDE_DIR})

#Find Qt (just for linking; if you're #including anything you might have to change this).
FIND_PACKAGE(Qt4 REQUIRED)

#Add all unit tests in addition to all source files.
add_executable(SM_UnitTests ${SharedCode_TEST} $<TARGET_OBJECTS:SimMob_Shared>)
 
#Link this executable.
target_link_libraries (SM_UnitTests ${LibraryList} ${CPPUNIT_LIBRARY})

#Additional link library; QT framework and QxCppUnit
target_link_libraries (SM_UnitTests ${QXCPPUNIT_LIBRARIES}  ${QT_LIBRARIES})
