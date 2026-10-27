# make          -> ./rlc        (computer, SDL2 from Homebrew: brew install sdl2)
# make miyoo    -> build/miyoo/rlc  (ARM binary for the Miyoo Mini; C++ runtime built in,
#                  glibc from the package's lib/: the toolchain's static glibc needs a
#                  newer kernel than the console's, "FATAL: kernel too old")
# make ev       -> regenerate the *EV.h / Types headers from ref/ (requires ref/)
CXXFLAGS = -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -D_DEFAULT_SOURCE
SRC = $(shell find src -name '*.cpp' ! -name 'platform_*.cpp')
MIYOO_CXX ?= $(HOME)/toolchains/armv7-unknown-linux-gnueabihf/bin/armv7-unknown-linux-gnueabihf-g++
MIYOO_FLAGS = -O3 -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard

rlc: $(SRC) $(shell find src -name '*.h')
	$(CXX) $(CXXFLAGS) $(shell sdl2-config --cflags) -o $@ $(SRC) src/platform_sdl.cpp $(shell sdl2-config --libs) -lm

miyoo: $(SRC) $(shell find src -name '*.h')
	mkdir -p build/miyoo
	$(MIYOO_CXX) $(CXXFLAGS) $(MIYOO_FLAGS) -static-libstdc++ -static-libgcc -o build/miyoo/rlc $(SRC) src/platform_miyoo.cpp -lm

ev:
	python3 tools/convert_ev.py ref/RogueLegacy1 src

clean:
	rm -rf rlc build

.PHONY: miyoo ev clean castletest

# Castle generation check (no graphics): ./castletest [seed] [count] [map.ppm]
castletest: tools/castletest.cpp src/RogueCastle/Levels/*.cpp src/RogueCastle/Levels/*.h
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ tools/castletest.cpp src/RogueCastle/Levels/*.cpp

# Heir generation check (no graphics): ./lineagetest [seed] [generations]
lineagetest: tools/lineagetest.cpp src/RogueCastle/Lineage/*.cpp src/RogueCastle/Lineage/*.h
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ tools/lineagetest.cpp src/RogueCastle/Lineage/*.cpp

# Manor check (no graphics): ./skilltest [lives] [gold per life]
skilltest: tools/skilltest.cpp src/RogueCastle/Skills/*.cpp src/RogueCastle/Skills/*.h
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ tools/skilltest.cpp src/RogueCastle/Skills/*.cpp

# Regenerate src/RogueCastle/Skills/SkillData.h from ref/
skills:
	python3 tools/convert_skills.py ref/RogueLegacy1 src

# Blacksmith / enchantress check (no graphics): ./equiptest [seed]
equiptest: tools/equiptest.cpp src/RogueCastle/Equipment/*.cpp src/RogueCastle/Equipment/*.h
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ tools/equiptest.cpp src/RogueCastle/Equipment/*.cpp

# Regenerate src/RogueCastle/Equipment/EquipmentData.h from ref/
equipment:
	python3 tools/convert_equipment.py ref/RogueLegacy1 src

# Save system check (no graphics): ./savetest
savetest: tools/savetest.cpp src/RogueCastle/Save/*.cpp src/RogueCastle/Lineage/*.cpp src/RogueCastle/Skills/*.cpp src/RogueCastle/Equipment/*.cpp
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ $^

# Font check (no window): ./texttest text.ppm
texttest: tools/texttest.cpp src/Text/*.cpp src/Text/*.h src/gfx.cpp
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ tools/texttest.cpp src/Text/*.cpp src/gfx.cpp

# Translation check: ./loctest [fr|en|de...]
loctest: tools/loctest.cpp src/Text/Loc.cpp src/Text/Loc.h src/Text/LocIDs.h
	$(CXX) -O2 -Wall -Wextra -std=c++17 -fno-exceptions -fno-rtti -o $@ tools/loctest.cpp src/Text/Loc.cpp

# Regenerate assets/lang/*.txt and src/Text/LocIDs.h from ref/
loc:
	python3 tools/convert_loc.py ref/RogueLegacy1 assets src

# Heir choice screen check (no window): ./lineagescreentest out -> out1.ppm out2.ppm
lineagescreentest: tools/lineagescreentest.cpp $(SRC) $(shell find src -name "*.h")
	$(CXX) $(CXXFLAGS) $(shell sdl2-config --cflags) -o $@ tools/lineagescreentest.cpp $(filter-out src/main.cpp,$(SRC)) src/platform_sdl.cpp $(shell sdl2-config --libs) -lm

# Manor screen check (no window): ./manortest out -> out1.ppm out2.ppm
manortest: tools/manortest.cpp $(SRC) $(shell find src -name "*.h")
	$(CXX) $(CXXFLAGS) $(shell sdl2-config --cflags) -o $@ tools/manortest.cpp $(filter-out src/main.cpp,$(SRC)) src/platform_sdl.cpp $(shell sdl2-config --libs) -lm
