# Locate Xsd from code synthesis include paths and binary
# Xsd can be found at http://codesynthesis.com/products/xsd/
# Written by Frederic Heem, frederic.heem _at_ telsey.it

# This module defines
# XSD_INCLUDE_DIR, where to find elements.hxx, etc.
# XSD_EXECUTABLE, where is the xsd compiler
# XSD_FOUND, If false, don't try to use xsd
FIND_PATH(XSD_INCLUDE_DIR 
  NAMES
    xsd/cxx/parser/elements.hxx
    cxx/parser/elements.hxx
  PATHS
    "[HKEY_CURRENT_USER\\software\\xsd\\include]"
    "[HKEY_CURRENT_USER]\\xsd\\include]"
    $ENV{XSDDIR}/include
    /usr/local/include
    /usr/include
    /Users/Shared/xsd_osxbin/libxsd
)

FIND_PROGRAM(XSD_EXECUTABLE 
  NAMES 
    xsd
  PATHS
    "[HKEY_CURRENT_USER\\xsd\\bin"
    $ENV{XSDDIR}/bin 
    /usr/local/bin
    /usr/bin
    /Users/Shared/xsd_osxbin/bin
)

# if the include and the program are found then we have it
IF(XSD_INCLUDE_DIR)
  IF(XSD_EXECUTABLE)
    SET( XSD_FOUND "YES" )
    MESSAGE(STATUS "XSD found.")
  ELSE(XSD_EXECUTABLE)
    MESSAGE(STATUS "XSD Not found .")
  ENDIF(XSD_EXECUTABLE)
ENDIF(XSD_INCLUDE_DIR)

MARK_AS_ADVANCED(
  XSD_INCLUDE_DIR
  XSD_EXECUTABLE
) 
