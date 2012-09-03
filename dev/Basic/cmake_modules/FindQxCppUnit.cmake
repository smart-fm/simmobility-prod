#
# Based on FindCppUnit; this finds QxCppUnit and QxRunner
#
# It sets the following variables:
#  QXCPPUNIT_FOUND       - Set to false, or undefined, if QxCppUnit isn't found.
#  QXCPPUNIT_INCLUDE_DIR - The QxCppUnit include directory.
#  QXCPPUNIT_LIBRARIES   - The QxCppUnit libraries to link against.

FIND_PATH(QXCPPUNIT_INCLUDE_DIR qxcppunit/testrunner.h)

FIND_LIBRARY(QXCPPUNIT_LIBRARY_1 NAMES qxcppunitd)
FIND_LIBRARY(QXCPPUNIT_LIBRARY_2 NAMES qxrunnerd)
set(QXCPPUNIT_LIBRARIES ${QXCPPUNIT_LIBRARY_1} ${QXCPPUNIT_LIBRARY_2})

IF (QXCPPUNIT_INCLUDE_DIR AND QXCPPUNIT_LIBRARY_1 AND QXCPPUNIT_LIBRARY_2)
   SET(QXCPPUNIT_FOUND TRUE)
ENDIF (QXCPPUNIT_INCLUDE_DIR AND QXCPPUNIT_LIBRARY_1 AND QXCPPUNIT_LIBRARY_2)

IF (QXCPPUNIT_FOUND)
   # show which QxCppUnit was found only if not quiet
   IF (NOT QxCppUnit_FIND_QUIETLY)
      MESSAGE(STATUS "Found QxCppUnit: ${QXCPPUNIT_LIBRARIES}")
   ENDIF (NOT QxCppUnit_FIND_QUIETLY)

ELSE (QXCPPUNIT_FOUND)

   # fatal error if QxCppUnit is required but not found
   IF (QxCppUnit_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find QxCppUnit")
   ENDIF (QxCppUnit_FIND_REQUIRED)

ENDIF (QXCPPUNIT_FOUND)
