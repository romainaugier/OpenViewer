@echo off
rem fresh build of the project

if exist build\ (
	rmdir build
) else (
	mkdir build
)
@echo on

cmake -S . -B build
cd build
cmake --build . --config Release