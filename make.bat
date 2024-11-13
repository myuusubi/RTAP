@echo off
cd %~dp0

if not exist "build" ( mkdir "build" )

nmake /NOLOGO /F Makefile.win %*