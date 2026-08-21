@echo off
REM Usage: compile.bat compile   - build QuizRelease.exe
REM        compile.bat clean     - remove intermediate object/resource files

setlocal EnableExtensions

set "MINGW_BIN=C:\Program Files (x86)\Dev-Cpp\MinGW64\bin"
set "BIN=QuizRelease.exe"
set "CXXFLAGS=-g3 -finput-charset=utf-8"
set "LIBS=-static-libgcc -lgdi32 -g3"
set "OBJS=QuizRelease.o Menu.o CreativeCommons.o QuizCommon.o"
set "RES=QuizRelease_private.res"

if /I "%~1"=="compile" goto :do_compile
if /I "%~1"=="clean" goto :do_clean

echo Usage: %~nx0 compile^|clean
echo   compile  - build %BIN%
echo   clean    - remove intermediate object/resource files
exit /b 1

:do_compile
where g++.exe >nul 2>nul
if %errorlevel% equ 0 goto :compilerfound

if not exist "%MINGW_BIN%\g++.exe" goto :nocompiler
set "PATH=%MINGW_BIN%;%PATH%"
goto :compilerfound

:nocompiler
echo [ERROR] g++.exe was not found in PATH or at "%MINGW_BIN%".
echo Install Dev-C++ or add your MinGW bin directory to PATH.
exit /b 1

:compilerfound
echo Compiling QuizRelease.cpp ...
g++.exe -c QuizRelease.cpp -o QuizRelease.o %CXXFLAGS%
if errorlevel 1 goto :builderror

echo Compiling Menu.cpp ...
g++.exe -c Menu.cpp -o Menu.o %CXXFLAGS%
if errorlevel 1 goto :builderror

echo Compiling CreativeCommons.cpp ...
g++.exe -c CreativeCommons.cpp -o CreativeCommons.o %CXXFLAGS%
if errorlevel 1 goto :builderror

echo Compiling QuizCommon.cpp ...
g++.exe -c QuizCommon.cpp -o QuizCommon.o %CXXFLAGS%
if errorlevel 1 goto :builderror

echo Compiling resources ...
REM --preprocessor is set explicitly to work around windres failing to
REM quote its default preprocessor command when MinGW is installed under
REM a path containing spaces (e.g. "Program Files (x86)").
windres.exe --preprocessor="gcc.exe -E -xc-header -DRC_INVOKED" -i QuizRelease_private.rc --input-format=rc -o %RES% -O coff
if errorlevel 1 goto :builderror

echo Linking %BIN% ...
g++.exe %OBJS% %RES% -o %BIN% %LIBS%
if errorlevel 1 goto :builderror

echo.
echo Build succeeded: %BIN%
exit /b 0

:builderror
echo.
echo [ERROR] Build failed.
exit /b 1

:do_clean
echo Cleaning intermediate build files ...
del /q %OBJS% %RES% >nul 2>nul
echo Done.
exit /b 0
