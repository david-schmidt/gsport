@echo off
REM
REM GSport - Windows startup batch file
REM

SET GSPORT_HOME=%CD%\
set CYGWIN=nodosfilewarning

:add_classpath
SET CWD=%CD%
if "%GSPORT_PATH_SET%" == "1" goto start
set GSPORT_PATH_SET=1
PATH=%PATH%;%GSPORT_HOME%lib

:start
GSport.exe
