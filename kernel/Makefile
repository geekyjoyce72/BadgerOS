
# SPDX-License-Identifier: MIT

MAKEFLAGS         += --silent
IDF_PATH          ?= $(shell pwd)/../esp-idf
SHELL             := /usr/bin/env bash
OUTPUT            ?= $(shell pwd)/firmware
BUILDDIR          ?= build
BADGER_RAMFS_ROOT ?= ../files/root
BADGEROS_PORT     ?= esp32c6
CROSS_COMPILE     ?= riscv64-unknown-linux-gnu-

include port/$(BADGEROS_PORT)/port.mk

.PHONY: configure
configure:
	mkdir -p '$(BUILDDIR)'
	touch '$(BUILDDIR)/fs_root.c'
	cmake -B '$(BUILDDIR)' "-DCMAKE_BUILD_TYPE=$${CMAKE_BUILD_TYPE:-Debug}"

.PHONY: build
build: configure
	../tools/ramfs-gen.py '$(BADGER_RAMFS_ROOT)' '$(BUILDDIR)/fs_root.c' init_ramfs
	cmake --build '$(BUILDDIR)'
	cmake --install '$(BUILDDIR)' --prefix '$(OUTPUT)'

.PHONY: clean
clean:
	rm -rf "$(BUILDDIR)"

.PHONY: clang-format-check
clang-format-check: configure
	echo "clang-format check the following files:"
	jq -r '.[].file' build/compile_commands.json | grep '\.[ch]$$'
	echo "analysis results:"
	clang-format-18 --dry-run $(shell jq -r '.[].file' build/compile_commands.json | grep '\.[ch]$$')

.PHONY: clang-tidy-check
clang-tidy-check: configure
	echo "clang-tidy check the following files:"
	jq -r '.[].file' build/compile_commands.json | grep '\.[ch]$$'
	echo "analysis results:"
	clang-tidy-18 -p build $(shell jq -r '.[].file' build/compile_commands.json | grep '\.[ch]$$') --warnings-as-errors="*"

.PHONY: gdb
gdb:
	$(CROSS_COMPILE)gdb -x port/$(BADGEROS_PORT)/gdbinit build/badger-os.elf
