CC       = gcc
BUILD   ?= debug

ifeq ($(BUILD),release)
CFLAGS   = -O3 -Wall -Wextra -Isrc -I../shit-game-engine/include $(shell pkg-config sdl3 --cflags) -DVERSION_SHA=\"$(GIT_SHA)\"
else
CFLAGS   = -g -Wall -Wextra -Isrc -I../shit-game-engine/include $(shell pkg-config sdl3 --cflags) -DVERSION_SHA=\"$(GIT_SHA)\"
endif

LDFLAGS  = -lSDL3 -lm $(shell pkg-config sdl3 --libs)
SRC      = $(wildcard src/*.c)
OBJ      = $(SRC:src/%.c=build/%.o)
ENGINE   = ../shit-game-engine/libsge.a
GIT_SHA = $(shell git rev-parse --short HEAD)

sick-ass-cube: $(OBJ) $(ENGINE)
	$(CC) $(OBJ) $(ENGINE) $(LDFLAGS) -o bin/sick-ass-cube

build/%.o: src/%.c
	mkdir -p build
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

$(ENGINE):
	$(MAKE) -C ../shit-game-engine BUILD=$(BUILD)

clean:
	rm -rf build bin sick-ass-cube