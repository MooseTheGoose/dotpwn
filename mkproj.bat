@echo OFF

cl corbasic.cpp CorGuids.lib MSCorEE.lib
echo.
csc HelloWorld.cs
echo.