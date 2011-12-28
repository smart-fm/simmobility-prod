## Find SOCI - PostgreSQL
##
## Copyright (c) 2010 Jamie Jones <jamie_jones_au@yahoo.com.au>
## This file is licensed under the GNU GPLv2 or any later versions,
##
## Copyright (c) 2011 Seth Hetu <seth.hetu@gmail.com>
##
## This module defines
## SOCIPOSTGRESQL_LIBRARIES
## SOCIPOSTGRESQL_FOUND, if false, do not try to link to SOCI-postgresql
## SOCIPOSTGRESQL_INCLUDE_DIRS, where to find the headers
##


if (SOCIPOSTGRESQL_INCLUDE_DIRS AND SOCIPOSTGRESQL_LIBRARIES)
  #Do nothing; the cached values of these already exist

else ()

  FIND_PATH(SOCIPOSTGRESQL_INCLUDE_DIRS soci-postgresql.h
      ${SOCI_DIR}/include
      ${SOCI_DIR}/include/soci
      $ENV{SOCI_DIR}/include
      $ENV{SOCI_DIR}/include/soci
      $ENV{SOCI_DIR}
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local/include
      /usr/local/include/soci/postgresql
      /usr/include
      /usr/include/soci/postgresql
      /sw/include # Fink
      /opt/local/include # DarwinPorts
      /opt/csw/include # Blastwave
      /opt/include
  )

  FIND_LIBRARY(SOCIPOSTGRESQL_LIBRARIES
      NAMES soci_postgresql  soci_postgresql-gcc-3_0
      PATHS
      ${SOCI_DIR}/lib
      $ENV{SOCI_DIR}/lib
      $ENV{SOCI_DIR}
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local/lib
      /usr/lib
      /sw/lib
      /opt/local/lib
      /opt/csw/lib
      /opt/lib
  )

  IF(SOCIPOSTGRESQL_LIBRARIES)
      IF (NOT SOCIPOSTGRESQL_FIND_QUIETLY)
      MESSAGE(STATUS "Found the SOCI PostgreSQL library at ${SOCIPOSTGRESQL_LIBRARIES}")
      MESSAGE(STATUS "Found the SOCI PostgreSQL headers at ${SOCIPOSTGRESQL_INCLUDE_DIRS}")
      ENDIF (NOT SOCIPOSTGRESQL_FIND_QUIETLY)
  ENDIF(SOCIPOSTGRESQL_LIBRARIES)

  IF(NOT SOCIPOSTGRESQL_LIBRARIES)
      IF (SOCIPOSTGRESQL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "SOCI PostgreSQL Not Found.")
      ENDIF (SOCIPOSTGRESQL_FIND_REQUIRED)
  ENDIF(NOT SOCIPOSTGRESQL_LIBRARIES)

endif (SOCIPOSTGRESQL_INCLUDE_DIRS AND SOCIPOSTGRESQL_LIBRARIES)

