# usage: make example1
#        make example1
#        make
#        make clean

CC = gcc
CFLAGS = -I../src -I../src/SDL2 -DSDL_MAIN_HANDLED -Wl,--subsystem,windows
LDFLAGS = -L../lib -lSDL2main -lSDL2 -lSDL2_ttf
TARGETS = example1 example2

% : %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.PHONY : all
all : $(TARGETS)

clean : 
	rm $(addsuffix .exe,$(TARGETS))
