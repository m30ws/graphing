# usage: make example1
#        make example1
#        make
#        make clean

CC = gcc
TARGETS = example1 example2

ifeq ($(OS),Windows_NT)
CFLAGS = -I../src -DSDL_MAIN_HANDLED -Wl,--subsystem,windows
LDFLAGS = -L../lib/win_x86 -lSDL2main -lSDL2 -lSDL2_ttf
else
CFLAGS = -I/usr/include/SDL2 -I../src -DSDL_MAIN_HANDLED
LDFLAGS = -lSDL2 -lSDL2_ttf
endif

% : %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.PHONY : all
all : $(TARGETS)

clean : 
	rm $(addsuffix .exe,$(TARGETS))
