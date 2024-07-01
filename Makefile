
# SPDX-License-Identifier: MIT

MAKEFLAGS += --silent
SHELL    := /usr/bin/env bash

.PHONY: all prepare openocd gdb flash monitor clean build clang-format-check clang-tidy-check

all: build

prepare:
	python -m venv .venv
	.venv/bin/pip install esptool
	git submodule update --init

build:
	$(MAKE) -C files build
	$(MAKE) -C kernel build

clean:
	$(MAKE) -C files clean
	$(MAKE) -C kernel clean

openocd:
	$(MAKE) -C kernel openocd

gdb:
	$(MAKE) -C kernel gdb

qemu:
	$(MAKE) -C kernel qemu

flash:
	$(MAKE) -C files build
	$(MAKE) -C kernel flash

image:
	$(MAKE) -C files build
	$(MAKE) -C kernel image

monitor:
	$(MAKE) -C kernel monitor

clang-format-check:
	$(MAKE) -C kernel clang-format-check

clang-tidy-check:
	$(MAKE) -C kernel clang-tidy-check
