
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.10.0)

set(target_arch "rv32imac_zicsr_zifencei")
set(target_abi "ilp32")

set(port_src
	${CMAKE_CURRENT_LIST_DIR}/src/entrypoint.S
	
	${CMAKE_CURRENT_LIST_DIR}/src/clkconfig.c
	${CMAKE_CURRENT_LIST_DIR}/src/gpio.c
	${CMAKE_CURRENT_LIST_DIR}/src/i2c.c
	${CMAKE_CURRENT_LIST_DIR}/src/interrupt.c
	${CMAKE_CURRENT_LIST_DIR}/src/intmtx.c
    ${CMAKE_CURRENT_LIST_DIR}/src/memprotect.c
	${CMAKE_CURRENT_LIST_DIR}/src/rawprint.c
	${CMAKE_CURRENT_LIST_DIR}/src/time.c
)
set(port_include ${CMAKE_CURRENT_LIST_DIR}/include)
set(port_link -T${CMAKE_CURRENT_LIST_DIR}/linker.ld)
set(port_flags)
