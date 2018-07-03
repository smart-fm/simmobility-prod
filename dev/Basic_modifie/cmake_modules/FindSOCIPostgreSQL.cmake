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
      C:/MinGW/include
      C:/MinGW/include/soci
      C:/MinGW/include/soci/postgresql
      C:/MinGW32/include
      C:/MinGW32/include/soci
      C:/MinGW32/include/soci/postgresql
      ~/Library/Frameworks
      /home/xuyan/xuyan_SimMobility/soci/include
      /home/xuyan/xuyan_SimMobility/soci/include/soci
      /Library/Frameworks
      /usr/local/include
      /usr/local/include/soci/postgresql
      /usr/include
      /usr/include/soci/postgresql
      /usr/local/include/soci/postgresql/
      /sw/include # Fink
      /opt/local/include # DarwinPorts
      /opt/local/include/soci/postgresql # MacPorts location
      /opt/csw/include # Blastwave
      /opt/include
  )

  #Hope the user isn't doing something risky (like installing 3_0 of soci and 3_1 of soci_pg)
  FIND_LIBRARY(SOCIPOSTGRESQL_LIBRARIES
      NAMES soci_postgresql  soci_postgresql-gcc-3_0  soci_postgresql_3_0  soci_postgresql-gcc-3_1  soci_postgresql_3_1
      PATHS
      ${SOCI_DIR}/lib
      $ENV{SOCI_DIR}/lib
      $ENV{SOCI_DIR}
      C:/MinGW/lib
      C:/MinGW32/lib
      ~/Library/Frameworks
      /home/xuyan/xuyan_SimMobility/soci/lib64
      /Library/Frameworks
      /usr/local/lib
      /usr/local/lib64
      /afs/csail.mit.edu/u/x/xuyan/lib/soci/soci-3.2.1/lib64
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

