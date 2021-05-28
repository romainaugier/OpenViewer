@echo off
rem build the project with CMake

if exist build\ (
	rmdir build
) else (
	mkdir build
)
@echo on

cmake -S . -B build
cd build
cmake --build . --config Release