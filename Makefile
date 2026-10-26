# make          -> ./rlc        (computer, SDL2 from Homebrew: brew install sdl2)
# make miyoo    -> build/miyoo/rlc  (static ARM binary for the Miyoo Mini)
# make ev       -> regenerate the *EV.h / Types headers from ref/ (requires ref/)
CXXFLAGS = -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -D_DEFAULT_SOURCE
SRC = $(shell find src -name '*.cpp' ! -name 'platform_*.cpp')
MIYOO_CXX ?= $(HOME)/toolchains/armv7-unknown-linux-gnueabihf/bin/armv7-unknown-linux-gnueabihf-g++
MIYOO_FLAGS = -O3 -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard

rlc: $(SRC) $(shell find src -name '*.h')
	$(CXX) $(CXXFLAGS) $(shell sdl2-config --cflags) -o $@ $(SRC) src/platform_sdl.cpp $(shell sdl2-config --libs) -lm

miyoo: $(SRC) $(shell find src -name '*.h')
	mkdir -p build/miyoo
	$(MIYOO_CXX) $(CXXFLAGS) $(MIYOO_FLAGS) -static -o build/miyoo/rlc $(SRC) src/platform_miyoo.cpp -lm

ev:
	python3 tools/convert_ev.py ref/RogueLegacy1 src

clean:
	rm -rf rlc build

.PHONY: miyoo ev clean
