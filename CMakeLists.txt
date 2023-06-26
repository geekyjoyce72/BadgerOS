
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.10.0)

# Set the C compiler.
set(CMAKE_C_COMPILER riscv32-unknown-linux-gnu-gcc)
set(CMAKE_C_COMPILER_FORCED true)

add_compile_options(-march=rv32imac_zifencei_zicsr -mabi=ilp32 -nodefaultlibs -O2)
add_link_options(-march=rv32imac_zifencei_zicsr -mabi=ilp32 -nodefaultlibs -nostartfiles -T${CMAKE_CURRENT_LIST_DIR}/linker.ld)

# For IDE users.
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

project(badgeros C ASM)

# Define executable file.
set(target main.elf)
add_executable(${target}
	${CMAKE_CURRENT_LIST_DIR}/src/log.c
	${CMAKE_CURRENT_LIST_DIR}/src/main.c
)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

# Include port-specific.
include(cpu/rv32imac/CMakeLists.txt)
include(port/esp32c6/CMakeLists.txt)
