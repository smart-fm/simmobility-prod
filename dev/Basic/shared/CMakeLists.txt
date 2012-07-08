#Build unit tests, if required.
IF (${BUILD_TESTS} MATCHES "ON")
	add_subdirectory(unit-tests)
ENDIF (${BUILD_TESTS} MATCHES "ON")
  
