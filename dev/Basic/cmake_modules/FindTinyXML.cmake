## - Find TinyXML
## Find the native TinyXML includes and library
##
## Copyright (c) 2010 Jamie Jones <jamie_jones_au@yahoo.com.au>
## This file is licensed under the GNU GPLv2 or any later versions,
##
## Copyright (c) 2011 Seth Hetu <seth.hetu@gmail.com>
##
## This module defines
##   TINYXML_FOUND       - True if TinyXML found.
##   TINYXML_INCLUDE_DIR - where to find tinyxml.h, etc.
##   TINYXML_LIBRARIES   - List of libraries when using TinyXML.
##


if (TINYXML_INCLUDE_DIR AND TINYXML_LIBRARIES)
  #Do nothing; the cached values of these already exist

else ()

  FIND_PATH(TINYXML_INCLUDE_DIR tinyxml.h
      ${TINYXML_DIR}/include
      $ENV{TINYXML_DIR}/include
      $ENV{TINYXML_DIR}
      C:/MinGW/include
      C:/MinGW32/include
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local/include
      /usr/include
      /sw/include # Fink
      /opt/local/include # DarwinPorts
      /opt/csw/include # Blastwave
      /opt/include
  )

  FIND_LIBRARY(TINYXML_LIBRARIES
      NAMES tinyxml
      PATHS
      ${TINYXML_DIR}/lib
      $ENV{TINYXML_DIR}/lib
      $ENV{TINYXML_DIR}
      C:/MinGW/lib
      C:/MinGW32/lib
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local/lib
      /usr/local/lib64
      /usr/lib
      /sw/lib
      /opt/local/lib
      /opt/csw/lib
      /opt/lib
  )

  IF(TINYXML_LIBRARIES)
      IF (NOT TINYXML_FIND_QUIETLY)
      MESSAGE(STATUS "Found the TinyXML library at ${TINYXML_LIBRARIES}")
      MESSAGE(STATUS "Found the TinyXML headers at ${TINYXML_INCLUDE_DIR}")
      ENDIF ()
  ENDIF()

  IF(NOT TINYXML_LIBRARIES)
      IF (TINYXML_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "TinyXML Not Found.")
      ENDIF ()
  ENDIF()
endif ()

