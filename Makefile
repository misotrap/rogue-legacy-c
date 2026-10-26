# make          -> ./rlc        (computer, SDL2 from Homebrew: brew install sdl2)
# make miyoo    -> build/miyoo/rlc  (static ARM binary for the Miyoo Mini)
SRC = src/main.c src/gfx.c src/sprites.c
CFLAGS = -O2 -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE
MIYOO_CC ?= $(HOME)/toolchains/armv7-unknown-linux-gnueabihf/bin/armv7-unknown-linux-gnueabihf-gcc
MIYOO_FLAGS = -O3 -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard

rlc: $(SRC) src/platform_sdl.c src/*.h
	$(CC) $(CFLAGS) $(shell sdl2-config --cflags) -o $@ $(SRC) src/platform_sdl.c $(shell sdl2-config --libs) -lm

miyoo: $(SRC) src/platform_miyoo.c src/*.h
	mkdir -p build/miyoo
	$(MIYOO_CC) $(CFLAGS) $(MIYOO_FLAGS) -static -o build/miyoo/rlc $(SRC) src/platform_miyoo.c -lm

clean:
	rm -rf rlc build

.PHONY: miyoo clean

# Castle generation check (no graphics): ./castletest [seed] [count] [map.ppm]
castletest: tools/castletest.cpp src/RogueCastle/Levels/*.cpp src/RogueCastle/Levels/*.h
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ tools/castletest.cpp src/RogueCastle/Levels/*.cpp
