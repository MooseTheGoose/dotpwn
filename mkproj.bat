@echo OFF

cl corinterop.cpp CorGuids.lib MSCorEE.lib
echo.
csc Test.cs