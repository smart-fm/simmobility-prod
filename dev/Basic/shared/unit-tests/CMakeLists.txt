#TODO: We can definitely automate this more.

SET(UnitTestLibs "")

#Find CPP Unit
find_package(CppUnit REQUIRED)
include_directories(${CPPUNIT_INCLUDE_DIR})
LIST(APPEND UnitTestLibs ${CPPUNIT_LIBRARY})

#Find QxCppUnit and Qt, both of which are optional.
find_package(QxCppUnit)
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui)

#For our Windows/OSX users (where QxRunner is not-so-supported), allow the 
#  tests to be build with pure CppUnit if either QxCppUnit or Qt4 are not found.
IF (QXCPPUNIT_FOUND AND QT4_FOUND)
  SET(SIMMOB_USE_TEST_GUI ON)
ELSE ()
  MESSAGE("Missing Qt4 or QxCppUnit: Unit Tests will be built in console mode.")
  SET(SIMMOB_USE_TEST_GUI OFF)
ENDIF (QXCPPUNIT_FOUND AND QT4_FOUND)


#Include/Library definitions for the GUI
IF (${SIMMOB_USE_TEST_GUI} MATCHES "ON")
  #Include QxCppUnit
  include_directories(${QXCPPUNIT_INCLUDE_DIR})
  LIST(APPEND UnitTestLibs ${QXCPPUNIT_LIBRARIES})

  #Qt is picky about how it's #included
  INCLUDE(${QT_USE_FILE})
  ADD_DEFINITIONS(${QT_DEFINITIONS})
  LIST(APPEND UnitTestLibs ${QT_LIBRARIES})
ENDIF (${SIMMOB_USE_TEST_GUI} MATCHES "ON")


#Add all unit tests in addition to all source files.
add_executable(SM_UnitTests ${SharedCode_TEST} $<TARGET_OBJECTS:SimMob_Shared>)
 
#Link this executable.
target_link_libraries (SM_UnitTests ${LibraryList}  ${UnitTestLibs})

