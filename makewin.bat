rem    If you're unable to run gnu make (e.g. cannot install Cygwin/MSYS)
rem    these commands should do the same as what Makefile does.

rem    `make demo` is equivalent to `make && make demon` which is what this
rem    script will do. Either simply run it or run commands individually.

@echo off

rem    `make`
g++ -o build/win_x86/graphing.exe src/graphing.cpp src/cJSON/*.c -I./src -L./lib/win_x86 -lSDL2main -lSDL2 -lSDL2_ttf -std=c++11 -O3 -Wl,--subsystem,windows

rem    `make demon`
cp build/win_x86/graphing.exe ./demo/win_x86/
cd ./demo/win_x86/ && py graphing.py 7 && cd ..
