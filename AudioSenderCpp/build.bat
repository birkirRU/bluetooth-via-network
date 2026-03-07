@echo off
set PATH=C:\msys64\ucrt64\bin;%PATH%
C:\msys64\ucrt64\bin\cmake.exe -G "Ninja" -DCMAKE_CXX_COMPILER=C:\msys64\ucrt64\bin\g++.exe -DCMAKE_C_COMPILER=C:\msys64\ucrt64\bin\gcc.exe -DCMAKE_MAKE_PROGRAM=C:\msys64\ucrt64\bin\ninja.exe -B build -S .
C:\msys64\ucrt64\bin\cmake.exe --build build
