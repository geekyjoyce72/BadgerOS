
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.10.0)

target_sources(${target} PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/src/int64.S
)
# target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
