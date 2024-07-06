# graphing
Utility for real-time data plotting onto multiple graphs in separate windows, with parameters adjustable to your needs. Data set may also be plotted all in one go. Uses its own header-library for managing individual graphs which utilizes SDL2 directly.

## 🖥 Features
Tool is comprised of two parts:
- main program that manages arbitrary amount windows with graphs on them
- header library responsible for creating and rendering windows and graphs

The program reads data from standard input, with each line representing y-values for a new point on each of the graphs, delimited by a specified delimiter.

Example, with delimiter being a comma:
```
5.0,13.28,255.4
```
The first graph will receive a new point with `y = 5.0`, the second one with `y = 13.28`, the third with `y = 255.4` and so on.

Some of the planned features for the future are loading parameters from a JSON config, some support for (auto)layouting of the windows, and its own network connection so there would be an option of receiving data directly instead of having to rely on external tools to pass network data to its standard input.

## ✨ Quick start

### Main program

To compile the program simply use `make` with no args. If you are on Windows and are unable to run GNU make, refer to `makewin.bat`.

You can also compile the code and run a sample python program that simulates a UDP server receiving data and forwards it to the compiled program in real-time.
```console
make demo
```
To just run demo with already compiled program:
```console
make demon
```
Tidy up with a simple:
```console
make clean
```

### Examples
There are also a few examples that illustrate usage of `graphing.h` in the `examples/` folder. To build examples simply position yourself and build all at once or one by one:
```console
make
make examples1
make examples2
make clean
```
The executables will simply reside in the same directory where they can be run. Alternatively you can simply inspect their sources.


## 🛠 Requirements
- SDL2 (and SDL2ttf)

`cJSON` and `uthash` libraries come included in appropriate directories. For Windows, SDL2 headers, its extensions and its `.dll` files also come included, while Linux version by default uses distro-provided SDL2. Only Windows and Linux are currently explicitly supported.

## 📊 Reference
The following reference refers to using the `graphing.h` itself.

Available functions:
```c
int graph_load();                                                 // Inits SDL+TTF (also sets SDL hint SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH); call
                                                                  //   once at the start of the program or none if you're doing it manually

void graph_shutdown();                                            // Destroys SDL+TFF (also deallocates caches)

graph_t* graph_create(graph_setup_t* setup);                      // Allocates a new graph on a new window
void graph_destroy(graph_t* graph);                               // Cleans up graph and closes its window
void graph_render(graph_t* graph);                                // Call in a loop to redraw points
void graph_addpoint(graph_t* graph, int x, float y);              // Set point at a specific x position
void graph_appendpoint(graph_t* graph, float y);                  // Append point to the end of the current list

graph_setup_t* graph_blank_setup();                               // Allocates a setup struct filled with default values
```

Available structs and constants:

```c
// bitmasks for need parameter for graph_finish_setup
#define GRAPHING_ID     0x8
#define GRAPHING_COLOR  0x4
#define GRAPHING_TITLE  0x2
#define GRAPHING_FONT   0x1
#define GRAPHING_ALL    0xF  // refers to "all 4 preceding parameters"
#define GRAPHING_NONE   0x0  // refers to "none of the preceding 4 parameters"

// predefined colors if not disabled
#define BLACK       0x00,0x00,0x00
#define WHITE       0xFF,0xFF,0xFF
#define RED         0xD0,0x00,0x00
#define GREEN       0x00,0xD0,0x00
#define BLUE        0x00,0x00,0xF0
#define YELLOW      0xFF,0xFF,0x00
#define LIGHT_GRAY  0xF0,0xF0,0xF0
#define PURPLE      0xD0,0x00,0xD0
#define GOLD        0xD1,0x8B,0x00

// main struct describing a single graph
typedef struct
{
    // Unique window id; may also be set manually
    int idx;

    // Window settings
    char* title;
    int width, height;
    int xpos, ypos;    // where on screen to place the newly created window

    // Graph settings
    int graph_amp;     // max y value that will be represented
    int graph_n;       // maximum of n points that will be on the graph at any time; x ∈ [0,n]
    int ysegments;     // amount of horizontal divider lines on y-axis between [0,graph_amp]
                       // try to keep an even number to avoid unwanted rounding issues
    int xsegments;     // amount of vertical divider lines on x-axis between [0,graph_n]

    int ymargin;
    int xmargin;

    // Font & color settings
    const char* font_path;
    int font_size;
    SDL_Color fg_color;
    SDL_Color bg_color;
    SDL_Color plot_color;

    // Control whether this struct gets freed after 1 use (default=yes)
    uint8_t consume;

} graph_setup_t;
```
