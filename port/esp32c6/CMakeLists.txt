
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.10.0)

target_sources(${target} PUBLIC
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
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
