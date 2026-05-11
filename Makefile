# Todo: fix so we are creating objects and not use just .c sources and recompile everything each time

ifeq ($(OS),Windows_NT)
# Compiled with: gcc-12.2.0-mingw-w64ucrt-10.0.0-r5 (win10)
# warning: A later version of the toolchain had a bug in the linker
#          which prevented the executable from being created and 
#          I do not know if it's still present in the newer versions

# Compiler & py binary (for demo)
CC = g++
PY = py

# Compile sources
SRCS = src/graphing.cpp src/cJSON/*.c

# Binary
BUILD_DIR = build/win_x86
OUT_FILE = graphing.exe

# Additional include dirs
INCLUDE_DIRS = -I./src

# Additional lib dirs
LIB_DIRS = -L./lib/win_x86

# Flags
CFLAGS = -std=c++11 -O3
LFLAGS = -lSDL2main -lSDL2 -lSDL2_ttf #-lSDL2_net

CFLAGS += -Wl,--subsystem,windows

DEBUG_FLAGS = # -g -Wl,--subsystem,console

# Misc
DEMO_DIR = demo/win_x86
DEMO_GRAPH_N_WINDOWS = 7 # 24 # 50 # :)

.PHONY : all clean demo demon

# Compile
all : 
	$(CC) $(SRCS) $(INCLUDE_DIRS) $(LIB_DIRS) $(CFLAGS) $(LFLAGS) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$(OUT_FILE)

# Demo, no compile
demon :
	cp $(BUILD_DIR)/$(OUT_FILE) ./$(DEMO_DIR)/
	cd ./$(DEMO_DIR)/ ; $(PY) graphing.py $(DEMO_GRAPH_N_WINDOWS) ; cd ../..

# Recompile, then demo
demo : all demon

# cls
clean :
	rm -f ./$(BUILD_DIR)/$(OUT_FILE)
	rm -f ./$(BUILD_DIR)/*.pdb


# Linux
else

# Compiler & py binary (for demo)
CC = g++
PY = python3

# Compile sources
SRCS = src/graphing.cpp src/cJSON/*.c

# Binary
BUILD_DIR = build/linux_x86
OUT_FILE = graphing

# Additional include dirs
INCLUDE_DIRS = -I/usr/include/SDL2 -I./src

# Additional lib dirs
LIB_DIRS = # -L./lib/linux_x86

# Flags
CFLAGS = -std=c++11 -O3 # -Wall
LFLAGS = -lSDL2 -lSDL2_ttf

DEBUG_FLAGS = # -g # -fno-inline 

# Misc
DEMO_DIR = demo/linux_x86
DEMO_GRAPH_N_WINDOWS = 24

.PHONY : all clean demo demon

# Compile
all : 
	$(CC) $(SRCS) $(INCLUDE_DIRS) $(LIB_DIRS) $(CFLAGS) $(LFLAGS) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$(OUT_FILE)

# Demo, no compile
demon :
	cp $(BUILD_DIR)/$(OUT_FILE) ./$(DEMO_DIR)/
	cd ./$(DEMO_DIR)/ ; $(PY) graphing.py $(DEMO_GRAPH_N_WINDOWS) ; cd ../..

# Recompile, then demo
demo : all demon

# cls
clean :
	rm -f ./$(BUILD_DIR)/$(OUT_FILE)
	rm -f ./$(BUILD_DIR)/*.pdb

endif
