BUILD_DIR ?= build
GENERATOR ?= Ninja

.PHONY: all configure build install clean

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -G $(GENERATOR)

build: configure
	cmake --build $(BUILD_DIR)

install: build
	./install.sh

clean:
	rm -rf $(BUILD_DIR)

