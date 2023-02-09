@echo off

rem Little utility batch script to build the library

echo Building OpenViewer

set BUILDTYPE=RelWithDebInfo
set RUNTESTS=0
set REMOVEOLDDIR=0

for %%x in (%*) do (
    call :ParseArg %%~x
)

if %REMOVEOLDDIR% equ 1 (
    if EXIST build (
        echo Removing old build directory
        rmdir /s /q build
    )
)

echo Build type : %BUILDTYPE%

if %RUNTESTS% equ 1 (
    cmake -S . -B build -DRUN_TESTS=1
)
else (
    cmake -S . -B build
)

cd build
cmake --build . --config %BUILDTYPE%

if %RUNTESTS% equ 1 ctest

cd ..

rem //////////////////////////////////
rem Little function to process args
:ParseArg

if "%~1" equ "Debug" (
    set BUILDTYPE=Debug
)

if "%~1" equ "debug" (
    set BUILDTYPE=Debug
)

if "%~1" equ "dbg" (
    set BUILDTYPE=Debug
)

rem Build and run tests
if "%~1" equ "RunTests" set RUNTESTS=1
if "%~1" equ "Tests" set RUNTESTS=1
if "%~1" equ "Test" set RUNTESTS=1
if "%~1" equ "runtests" set RUNTESTS=1
if "%~1" equ "tests" set RUNTESTS=1
if "%~1" equ "test" set RUNTESTS=1

if "%~1" equ "RemoveDir" set REMOVEOLDDIR=1
if "%~1" equ "Remove" set REMOVEOLDDIR=1
if "%~1" equ "remove" set REMOVEOLDDIR=1
if "%~1" equ "removedir" set REMOVEOLDDIR=1
if "%~1" equ "rmdir" set REMOVEOLDDIR=1
if "%~1" equ "rdir" set REMOVEOLDDIR=1

exit /B 0

rem //////////////////////////////////
