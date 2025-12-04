@echo off

g++ emulator.cpp fx_core.cpp -lgdi32 -luser32 -o emulator.exe

pause