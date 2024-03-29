PORT ?= /dev/ttyACM0
BUILDDIR ?= build
IDF_PATH ?= $(shell pwd)/esp-idf
IDF_EXPORT_QUIET ?= 0
SHELL := /usr/bin/env bash
DEVICE ?= default

.PHONY: prepare clean build flash erase monitor menuconfig

all: prepare build

prepare:
	git submodule update --init --recursive
	cd esp-idf; bash install.sh

clean:
	rm -rf "$(BUILDDIR)"

build:
	source "$(IDF_PATH)/export.sh" && idf.py build

install: prepare build
	python3 tools/webusb_push.py "Spwn" build/main.bin --run --address=$(DEVICE)

massinstall: prepare build
	./massinstall.sh

erase:
	source "$(IDF_PATH)/export.sh" && idf.py erase-flash -p $(PORT)

monitor:
	source "$(IDF_PATH)/export.sh" && idf.py monitor -p $(PORT)

menuconfig:
	source "$(IDF_PATH)/export.sh" && idf.py menuconfig
