@echo off
setlocal
set TOOL=aarch64-none-elf
set CFLAGS=-Wall -O2 -ffreestanding -nostdlib -Isrc
set CXXFLAGS=-std=c++17 -Wall -O2 -Isrc -DBARE_METAL -D_GLIBCXX_HOSTED=1

echo [1/6] Assembling start.S
"%TOOL%-gcc" %CFLAGS% -c src\start.S -o start.o || goto :error

echo [2/6] Compiling C sources
"%TOOL%-gcc" %CFLAGS% -c src\mailbox.c -o mailbox.o || goto :error
"%TOOL%-gcc" %CFLAGS% -c src\framebuffer.c -o framebuffer.o || goto :error
"%TOOL%-gcc" %CFLAGS% -c src\kernel_main.c -o kernel_main.o || goto :error
"%TOOL%-gcc" %CFLAGS% -c src\sys_stubs.c -o sys_stubs.o || goto :error

echo [3/6] Compiling C++ interpreter and port
"%TOOL%-g++" %CXXFLAGS% -c ..\fx_core.cpp -o fx_core.o || goto :error
"%TOOL%-g++" %CXXFLAGS% -c src\fx_port.cpp -o fx_port.o || goto :error

echo [4/6] Linking kernel8.elf
"%TOOL%-g++" -T link.ld -nostartfiles -Wl,--gc-sections -o kernel8.elf start.o mailbox.o framebuffer.o kernel_main.o sys_stubs.o fx_core.o fx_port.o -lstdc++ -lsupc++ -lgcc || goto :error

echo [5/6] Converting to kernel8.img
"%TOOL%-objcopy" kernel8.elf -O binary kernel8.img || goto :error

echo [6/6] Done
goto :eof

:error
echo Build failed.
exit /b 1

aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdlib -c src/start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdlib -c src/mailbox.c -o mailbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdlib -c src/framebuffer.c -o framebuffer.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdlib -c src/kernel_main.c -o kernel_main.o
aarch64-none-elf-g++ -std=c++17 -Wall -O2 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -c ..\\fx_core.cpp -o fx_core.o
aarch64-none-elf-g++ -std=c++17 -Wall -O2 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -c src/fx_port.cpp -o fx_port.o

aarch64-none-elf-ld -T ..\\link.ld -o kernel8.elf start.o mailbox.o framebuffer.o kernel_main.o fx_port.o fx_core.o

aarch64-none-elf-objcopy kernel8.elf -O binary kernel8.img

pause
