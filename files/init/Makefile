
# SPDX-License-Identifier: MIT

MAKEFLAGS += --silent
SHELL    := /usr/bin/env bash
PORT     ?= $(shell find /dev/ -name ttyUSB* -or -name ttyACM* | head -1)
OUTPUT   ?= $(shell pwd)/app
BUILDDIR ?= build

.PHONY: all clean-all clean build

all: build

build:
	mkdir -p '$(BUILDDIR)'
	cmake -B '$(BUILDDIR)'
	cmake --build '$(BUILDDIR)'
	cmake --install '$(BUILDDIR)' --prefix '$(OUTPUT)'

clean-all: clean
clean:
	rm -rf '$(BUILDDIR)' '$(OUTPUT)'
