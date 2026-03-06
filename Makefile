CC       = gcc
CFLAGS   = -g -Wall -Wextra -Isrc -I../shit-game-engine/include $(shell sdl2-config --cflags)
LDFLAGS  = $(shell sdl2-config --libs) -lm
SRC      = $(wildcard src/*.c)
OBJ      = $(SRC:src/%.c=build/%.o)
ENGINE   = ../shit-game-engine/libsge.a

sick-ass-cube: $(OBJ) $(ENGINE)
	$(CC) $(OBJ) $(ENGINE) $(LDFLAGS) -o bin/sick-ass-cube

build/%.o: src/%.c
	mkdir -p build
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

$(ENGINE):
	$(MAKE) -C ../shit-game-engine

clean:
	rm -rf build bin sick-ass-cube