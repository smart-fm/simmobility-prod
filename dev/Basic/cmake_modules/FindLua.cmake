## Find LUA
##
## This module defines
## LUA_LIBRARIES
## LUA_FOUND
## LUA_INCLUDE_DIRS, where to find the headers
##

if (LUA_INCLUDE_DIRS AND LUA_LIBRARIES)
  #Do nothing; the cached values of these already exist

else ()

  FIND_PATH(LUA_INCLUDE_DIRS lua.h
      /usr/local/include
      /usr/local/include/lua
      /usr/include
      /usr/include/lua
  )

  FIND_LIBRARY(LUA_LIBRARIES
      NAMES lua luac
      PATHS
      /usr/local/lib
      /usr/local/lib64
      /usr/lib
  )
  

  IF(LUA_LIBRARIES)
      IF (NOT LUA_FIND_QUIETLY)
      MESSAGE(STATUS "Found the LUA library at ${LUA_LIBRARIES}")
      MESSAGE(STATUS "Found the LUA headers at ${LUA_INCLUDE_DIRS}")
      ENDIF (NOT LUA_FIND_QUIETLY)
  ENDIF(LUA_LIBRARIES)

  IF(NOT LUA_LIBRARIES)
      IF (LUA_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "LUA Not Found.")
      ENDIF (LUA_FIND_REQUIRED)
  ENDIF(NOT LUA_LIBRARIES)

endif (LUA_INCLUDE_DIRS AND LUA_LIBRARIES)
