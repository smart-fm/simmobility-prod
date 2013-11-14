## Find MongoClient
##
## This module defines
## MONGOCLIENT_LIBRARY
## MONGOCLIENT_FOUND
## MONGOCLIENT_INCLUDE_DIRS, where to find the headers
##

if (MONGOCLIENT_INCLUDE_DIRS AND MONGOCLIENT_LIBRARY)
  #Do nothing; the cached values of these already exist

else ()

  FIND_PATH(MONGOCLIENT_INCLUDE_DIRS dbclient.h
      /usr/local/include
      /usr/local/include/mongo/client
      /usr/include
      /usr/include/mongo/client
  )

  FIND_LIBRARY(MONGOCLIENT_LIBRARY
      NAMES mongoclient
      PATHS
      /usr/local/lib
      /usr/local/lib64
      /usr/lib
  )
  

  IF(MONGOCLIENT_LIBRARY)
      IF (NOT MONGOCLIENT_FIND_QUIETLY)
      MESSAGE(STATUS "Found the MONGOCLIENT library at ${MONGOCLIENT_LIBRARY}")
      MESSAGE(STATUS "Found the MONGOCLIENT headers at ${MONGOCLIENT_INCLUDE_DIRS}")
      ENDIF (NOT MONGOCLIENT_FIND_QUIETLY)
  ENDIF(MONGOCLIENT_LIBRARY)

  IF(NOT MONGOCLIENT_LIBRARY)
      IF (MONGOCLIENT_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "MONGOCLIENT Not Found.")
      ENDIF (MONGOCLIENT_FIND_REQUIRED)
  ENDIF(NOT MONGOCLIENT_LIBRARY)

endif (MONGOCLIENT_INCLUDE_DIRS AND MONGOCLIENT_LIBRARY)
