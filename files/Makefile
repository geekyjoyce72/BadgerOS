
# SPDX-License-Identifier: MIT

MAKEFLAGS += --silent
SHELL    := /usr/bin/env bash
OUTPUT   ?= $(shell pwd)/root

.PHONY: all clean-all clean build

all: build

build:
	$(MAKE) -C init
	mkdir -p '$(OUTPUT)'
	mkdir -p '$(OUTPUT)/sbin'
	mkdir -p '$(OUTPUT)/etc'
	cp init/app/app.elf '$(OUTPUT)/sbin/init'
	echo -e 'Hello, World!\nCompiled at $(shell date)' > $(OUTPUT)/etc/motd

clean-all: clean
	$(MAKE) -C init clean-all

clean:
	rm -rf '$(OUTPUT)'
