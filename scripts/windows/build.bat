@echo off
rem build the project

cd ../../..
cmake -S . -B build
cd build
cmake --build . --config Release