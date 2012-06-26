#Build unit tests, if required.
IF (${BUILD_TESTS} MATCHES "ON")
	add_subdirectory(unit-tests)
ENDIF (${BUILD_TESTS} MATCHES "ON")

#Include all three directories, in case we need them.  
include_directories("${PROJECT_SOURCE_DIR}/short")
include_directories("${PROJECT_SOURCE_DIR}/medium")
include_directories("${PROJECT_SOURCE_DIR}/long")
