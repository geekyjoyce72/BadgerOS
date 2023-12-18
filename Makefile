
# SPDX-License-Identifier: MIT

MAKEFLAGS += --silent
SHELL    := /usr/bin/env bash
OUTPUT   ?= $(shell pwd)/root

.PHONY: all flash monitor clean-all clean build clang-format-check clang-tidy-check

all: build

build:
	$(MAKE) -C files build
	$(MAKE) -C kernel build

clean-all: clean
clean:
	$(MAKE) -C files clean-all
	$(MAKE) -C kernel clean-all

flash:
	$(MAKE) -C kernel flash

monitor:
	$(MAKE) -C kernel monitor

clang-format-check:
	$(MAKE) -C kernel clang-format-check

clang-tidy-check:
	$(MAKE) -C kernel clang-format-check
