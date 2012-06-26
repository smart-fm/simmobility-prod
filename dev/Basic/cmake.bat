rem This batch file override any calls from cmake within Eclipse, forcing the "MinGW" option.
rem This way, we can avoid using a "-G" option in Eclipse, and thus have the same Make targets
rem on both Windows and Linux.

@cmake.exe -G "MinGW Makefiles" %*
