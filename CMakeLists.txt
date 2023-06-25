cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_C_COMPILER riscv32-unknown-linux-gnu-gcc)
set(CMAKE_C_COMPILER_FORCED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

project(baremetalc6 C ASM)

add_compile_options(-nodefaultlibs -O2)
add_link_options(-nodefaultlibs -nostartfiles -T${CMAKE_CURRENT_LIST_DIR}/linker.ld)

add_executable(main.elf
	src/entrypoint.S
	src/int64.S
	
	src/clkconfig.c
	src/gpio.c
	src/i2c.c
	src/log.c
	src/main.c
	src/rawprint.c
	src/time.c
)
target_include_directories(main.elf PUBLIC include)
