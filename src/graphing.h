/*
 *	Copyright (c) 2023-2024 Fran Tomljenović
 * 
 *	Available defines:
 *	==================
 *	#define GRAPHING_NO_CACHING
 *	- by default uses uthash.h library to cache legend textures and fonts
 *	#define GRAPHING_USE_MUTEX
 *	- use a mutex when adding/appending new points to a graph (default: no)
 *	#define GRAPHING_USE_SYSTEM_SDL
 *	- by default includes SDL from src/SDL2/
 *	- define this to use #include <SDL2/SDL.h> instead of #include "SDL2/SDL.h"
 *	#define GRAPHING_NO_SRAND
 *	- do not initialize srand(); assumes you will do so manually elsewhere
 *	#define GRAPHING_NO_DEFINED_COLORS
 *	- do not define common constants such as BLACK, WHITE, RED... (default: it does.)
 * 
 */

#ifndef __GRAPHING_H_
#define __GRAPHING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifndef GRAPHING_USE_SYSTEM_SDL
	#include "SDL2/SDL.h"
	#include "SDL2/SDL_ttf.h"
	#include "SDL2/SDL_thread.h"
	// #include "SDL2/SDL_net.h"
	#ifdef GRAPHING_USE_MUTEX
		#include "SDL2/SDL_mutex.h"
	#endif
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_ttf.h>
	#include <SDL2/SDL_thread.h>
	// #include <SDL2/SDL_net.h>
	#ifdef GRAPHING_USE_MUTEX
		#include <SDL2/SDL_mutex.h>
	#endif
#endif

#ifndef GRAPHING_NO_CACHING
	#include "tdh/uthash.h"
#endif

#ifndef ARR_SIZE
#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#endif


/*
 *  GRAPH SETUP BITMASKS
 *
 *  Need bits:
 *  ==========
 *  x  x  x  x   I    C      T       F
 *  -  -  -  -  id  color  title  fontpath
 */
#define GRAPHING_ID     0x8
#define GRAPHING_COLOR  0x4
#define GRAPHING_TITLE  0x2
#define GRAPHING_FONT   0x1
#define GRAPHING_ALL    0xF
#define GRAPHING_NONE   0x0

const char GRAPHING_DEFAULT_TITLE[]         = "--";
const char GRAPHING_DEFAULT_FONT[]          = "selawk.ttf";
#define GRAPHING_DEFAULT_FONTSIZE           12

#ifndef GRAPHING_DEFAULT_WIDTH
#define GRAPHING_DEFAULT_WIDTH              640 // 240
#endif
#ifndef GRAPHING_DEFAULT_HEIGHT
#define GRAPHING_DEFAULT_HEIGHT             480 // 180
#endif

#ifndef GRAPHING_NO_DEFINED_COLORS
	#define BLACK                           0x00,0x00,0x00
	#define WHITE                           0xFF,0xFF,0xFF
	#define RED                             0xD0,0x00,0x00
	#define GREEN                           0x00,0xD0,0x00
	#define BLUE                            0x00,0x00,0xF0
	#define YELLOW                          0xFF,0xFF,0x00
	#define LIGHT_GRAY                      0xF0,0xF0,0xF0
	#define PURPLE     	                    0xD0,0x00,0xD0
	#define GOLD                            0xD1,0x8B,0x00

	#define GRAPHING_DEFAULT_BG_COLOR       LIGHT_GRAY
	#define GRAPHING_DEFAULT_FG_COLOR       BLACK
	#define GRAPHING_DEFAULT_PLOT_COLOR     PURPLE
#else
	#define GRAPHING_DEFAULT_BG_COLOR       0xF0,0xF0,0xF0
	#define GRAPHING_DEFAULT_FG_COLOR       0x00,0x00,0x00
	#define GRAPHING_DEFAULT_PLOT_COLOR     0xD0,0x00,0xC0
#endif

#define GRAPHING_DEFAULT_BG_ALPHA           0xFF
#define GRAPHING_DEFAULT_FG_ALPHA           0x00
#define GRAPHING_DEFAULT_PLOT_ALPHA         0xFF

#define GRAPHING_DEFAULT_TITLE_SIZE         32+1

#define GRAPHING_DEFAULT_AMP                1000
#define GRAPHING_DEFAULT_N                  500

#define GRAPHING_DEFAULT_YSEGMENTS          2
#define GRAPHING_DEFAULT_XSEGMENTS          5

#define GRAPHING_DEFAULT_XMARGIN            50
#define GRAPHING_DEFAULT_YMARGIN            10
#define GRAPHING_DEFAULT__EXTENDED_YMARGIN  20

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;

	char* title;
	int width;
	int height;
	int fps;
} Window;

typedef struct {
	SDL_Rect rect;
	SDL_Texture* texture;
} textpos_t;

typedef struct {
	Window window;

	int graph_amp;
	int graph_n;
	int ysegments;
	int xsegments;
	int ymargin;
	int xmargin;
	int _extended_ymargin;

	int xstart;
	int ystart;
	int xend;
	int yend;

	int zerox;
	int zeroy;
	int real_ysize;
	int real_xsize;
	int ysegment_size;
	int xsegment_size;
	int real_ysegment_size;
	int real_xsegment_size;

	TTF_Font* font;
	SDL_Color fg_color;
	SDL_Color bg_color;
	SDL_Color plot_color;
	SDL_Point sidebar_scale[4];
	int ysegment_count;
	textpos_t* ysegment_texts;

	int pos;
	float* points_f;
	SDL_FPoint* _render_points_f;

	#ifdef GRAPHING_USE_MUTEX
	SDL_mutex* mutex; //
	#endif

} graph_t;

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
                       //   try to keep an even number to avoid unwanted rounding issues
	int xsegments;     // amount of vertical divider lines on x-axis between [0,graph_n]

	int ymargin;
	int xmargin;
	int _extended_ymargin;

	// Font & color settings
	const char* font_path;
	int font_size;
	SDL_Color fg_color;
	SDL_Color bg_color;
	SDL_Color plot_color;

	// Control whether this struct gets freed after 1 use (default=yes)
	uint8_t consume;

} graph_setup_t;


#ifndef GRAPHING_NO_CACHING

	typedef struct font_entry_ {
		TTF_Font* font; // switch to using subtable for fontsizes (path repeats for each)?
		char path[1024+1];

		UT_hash_handle hh;
	} font_entry_t;

	font_entry_t* g_font_cache = NULL;

	typedef struct sscache_key_ {
		TTF_Font* font;
		SDL_Color color;
		// 10000000000
		// 3.246*10^-7
		// 12457.71834
		char label[11+1]; // 11 digits/chars + \0

	} /* __attribute__((packed)) */ sscache_key_t;

	typedef struct segmentsurf_entry_ {
		sscache_key_t key;
		SDL_Surface* surf;

		UT_hash_handle hh;
	} segmentsurf_entry_t;

	segmentsurf_entry_t* segmentsurf_cache = NULL;
#endif


// Public
int graph_load();                                                 // Inits SDL+TTF (also sets SDL hint SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH); call
                                                                  //   once at the start of the program or none if you're doing it manually

void graph_shutdown();                                            // Destroys SDL+TFF (also deallocates caches)

graph_t* graph_create(graph_setup_t* setup);                      // Allocates a new graph on a new window
void graph_destroy(graph_t* graph);                               // Cleans up graph and closes its window
void graph_render(graph_t* graph);                                // Call in a loop to redraw points
void graph_addpoint(graph_t* graph, int x, float y);              // Set point at a specific x position
void graph_appendpoint(graph_t* graph, float y);                  // Append point to the end of the current list

graph_setup_t* graph_blank_setup();                               // Allocates a setup struct filled with default values

// Deprecated
uint32_t graph_finish_setup(graph_setup_t* setup, uint8_t need);  // Fills missing fields with default values; 'need' is a bitmask of the fields 
                                                                  //   that need to be filled with default values but cannot be detected (only some
                                                                  //   of the fields need this, to see which ones look under Bitmasks).

// Internal
unsigned long graphing_random_id();
void graphing_srand();
//int randint(int high, int low=0);

TTF_Font* graph_open_default_font_();
TTF_Font* graph_open_font_(const char* fpath, int fsize, uint8_t fallback_to_default);
TTF_Font* graph_open_font(const char* fpath, int fsize);

void graph_init_render_(graph_t* graph);

int create_window(Window* w, char* title, int width, int height, int fps, int xpos, int ypos);
void close_window(Window* w);


/* */
void graphing_srand()
{
	#ifndef GRAPHING_NO_SRAND
		static uint8_t firsttime = 1;
		if (firsttime) {
			srand(time(NULL));
			firsttime = 0;
		}
	#else

	#endif
}


/*
int randint(int high, int low = 0)
{
	graphing_srand();
	return (int)((float)rand() / RAND_MAX * (high - low) + low);
}
*/


/* */
unsigned long graphing_random_id()
{
	/*struct timespec {
		time_t   tv_sec;        // seconds
		long     tv_nsec;       // nanoseconds
	};*/
	graphing_srand();
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	// printf("# sec: %lu\n# nsec: %lu\n", (unsigned long)ts.tv_sec, (unsigned long)ts.tv_nsec);
	return (unsigned long)ts.tv_sec + (unsigned long)ts.tv_nsec;
}


/* */
graph_setup_t* graph_blank_setup()
{
	graph_setup_t* data = (graph_setup_t*) malloc(sizeof(*data));

	*(data) = (graph_setup_t) {

		// Unique window id
		.idx                   = (int)graphing_random_id(),

		// Window settings
		.title                 = (char*)GRAPHING_DEFAULT_TITLE,
		.width                 = GRAPHING_DEFAULT_WIDTH,
		.height                = GRAPHING_DEFAULT_HEIGHT,
		.xpos                  = SDL_WINDOWPOS_UNDEFINED,
		.ypos                  = SDL_WINDOWPOS_UNDEFINED,

		// Graph settings
		.graph_amp             = GRAPHING_DEFAULT_AMP,
		.graph_n               = GRAPHING_DEFAULT_N,
		.ysegments             = GRAPHING_DEFAULT_YSEGMENTS,
		.xsegments             = GRAPHING_DEFAULT_XSEGMENTS,

		.ymargin               = GRAPHING_DEFAULT_YMARGIN,
		.xmargin               = GRAPHING_DEFAULT_XMARGIN,
		._extended_ymargin     = GRAPHING_DEFAULT__EXTENDED_YMARGIN,
		
		// Font & color settings
		.font_path             = (char*)GRAPHING_DEFAULT_FONT,
		.font_size             = GRAPHING_DEFAULT_FONTSIZE,
		.fg_color              = {GRAPHING_DEFAULT_FG_COLOR},
		.bg_color              = {GRAPHING_DEFAULT_BG_COLOR},
		.plot_color            = {GRAPHING_DEFAULT_PLOT_COLOR},

		// Control whether this struct gets freed after 1 use (default=YES)
		// Anything user-allocated is not freed !!
		.consume               = 1,
	};

	return data;
}


/*
 *
 *  [!] NOT recommended to use anymore, since it needs "valid" default values
 *      to know which fields need filling up. Those values you would get either
 *      by using graph_blank_setup() or by manually setting them, at which point
 *      it just doesn't make sense to use it.
 *
 *
 *  Need bits:
 *  ==========
 *  x  x  x  x   I    C      T       F
 *  -  -  -  -  id  color  title  fontpath
 * 
 * GRAPHING_ID     0x8
 * GRAPHING_COLOR  0x4
 * GRAPHING_TITLE  0x2
 * GRAPHING_FONT   0x1
 * GRAPHING_ALL    0xF
 * GRAPHING_NONE   0x0
 */
uint32_t graph_finish_setup(graph_setup_t* setup, uint8_t need)
{
	uint32_t copied = 0;
	graph_setup_t* blank = graph_blank_setup();

	// Unique window id
	if (need & GRAPHING_ID)
		setup->idx = blank->idx;

	// Window settings
	if (need & GRAPHING_TITLE || setup->title == NULL)
		setup->title = blank->title;

	if (setup->width < 1)
		setup->width = blank->width;

	if (setup->height < 1)
		setup->height = blank->height;

	if (setup->xpos < 0)
		setup->xpos = blank->xpos;

	if (setup->ypos < 0)
		setup->ypos = blank->ypos;

	// Graph settings
	if (setup->graph_amp < 1)
		setup->graph_amp = blank->graph_amp;

	if (setup->graph_n < 1)
		setup->graph_n = blank->graph_n;

	if (setup->ysegments < 0)
		setup->ysegments = blank->ysegments;

	if (setup->xsegments < 0)
		setup->xsegments = blank->xsegments;


	if (setup->ymargin < 0)
		setup->ymargin = blank->ymargin;

	if (setup->xmargin < 0)
		setup->xmargin = blank->xmargin;

	if (setup->_extended_ymargin < 0)
		setup->_extended_ymargin = blank->_extended_ymargin;

	// Font & color settings
	if (need & GRAPHING_FONT || setup->font_path == NULL)
		setup->font_path = blank->font_path;

	if (setup->font_size < 1)
		setup->font_size = blank->font_size;

	if (need & GRAPHING_COLOR)
		setup->plot_color = blank->plot_color;
	
	// Struct gets auto freed ?
	if (setup->consume < 0)
		setup->consume = blank->consume;

	free(blank);
	return copied;
}


/* */
TTF_Font* graph_open_default_font_()
{
	printf("opening default font %s (%d)\n", GRAPHING_DEFAULT_FONT, GRAPHING_DEFAULT_FONTSIZE);
	return TTF_OpenFont(GRAPHING_DEFAULT_FONT, GRAPHING_DEFAULT_FONTSIZE);
}
/* */
TTF_Font* graph_open_font_(const char* fpath, int fsize, uint8_t fallback_to_default)
{
	TTF_Font* res_fnt;
	#ifndef GRAPHING_NO_CACHING
		// Use if cached, otherwise load
		font_entry_t* font_buff;
		HASH_FIND_STR(g_font_cache, fpath, font_buff);
		if (!font_buff) {
			TTF_Font* new_font = TTF_OpenFont(fpath, fsize);
			if (new_font) {
				font_buff = (font_entry_t*) malloc(sizeof(*font_buff)); // font_entry_t*
				strncpy( font_buff->path, fpath, sizeof(font_buff->path) - 1 ); // account for \0
				font_buff->path[sizeof(font_buff->path) - 1] = '\0';
				font_buff->font = new_font;
				HASH_ADD_STR(g_font_cache, path, font_buff);

			/* else crash/use default font ?
			 * current solution: try to open default only if fpath != DEFAULT
			 * -> if default font fails, oh well...
			 */
			} else if (fallback_to_default) {
				printf("open font error: %s\n", TTF_GetError());
				if (strcmp(fpath, GRAPHING_DEFAULT_FONT) != 0) {
					res_fnt = graph_open_font_(fpath, fsize, 0);
				}
			}
		}
		res_fnt = font_buff->font;
	#else
		res_fnt = TTF_OpenFont(fpath, fsize);
		if (!res_fnt) {
			printf("open font error: %s\n", TTF_GetError());
		}
	#endif

	return res_fnt;
}


/* */
TTF_Font* graph_open_font(const char* fpath, int fsize)
{
	return graph_open_font_(fpath, fsize, 1);
}


/* */
graph_t* graph_create(graph_setup_t* setup)
{
	graph_t* graph = (graph_t*) malloc(sizeof(*graph)); // graph_t*
	
	int len = strlen(setup->title);
	len = (len > 128) ? 128 : len;
	
	graph->window.title = (char*) malloc(len + 1); // char*
	snprintf(graph->window.title, len + 1, "%s", setup->title);

	create_window(&graph->window, graph->window.title, setup->width, setup->height, -1, setup->xpos, setup->ypos);

	graph->graph_amp 	= setup->graph_amp;
	graph->graph_n 		= setup->graph_n;
	graph->xsegments 	= setup->xsegments;
	graph->ysegments 	= setup->ysegments; // Try to keep an even number to avoid unwanted rounding issues
	graph->xmargin 		= setup->xmargin;
	graph->ymargin 		= setup->ymargin;
	graph->_extended_ymargin = setup->_extended_ymargin;
	////
	graph->xstart 		= graph->xmargin;
	graph->ystart 		= graph->ymargin;

	graph->xend 		= graph->window.width - graph->xmargin;
	graph->yend 		= graph->window.height - graph->ymargin;

	graph->ystart 		+= graph->_extended_ymargin;
	graph->yend 		-= graph->_extended_ymargin;

	graph->zerox = graph->xstart;
	graph->zeroy = graph->ystart + (graph->yend - graph->ystart) / 2;

	// Setup sidebar_scale lines
	graph->sidebar_scale[0] = (SDL_Point){graph->xstart, graph->ystart - graph->_extended_ymargin};
	graph->sidebar_scale[1] = (SDL_Point){graph->xstart, graph->yend + graph->_extended_ymargin};
	graph->sidebar_scale[2] = (SDL_Point){graph->xstart, graph->zeroy};
	graph->sidebar_scale[3] = (SDL_Point){graph->xend, graph->zeroy};

	// Setup segments
	graph->real_ysize			= graph->yend - graph->ystart; // pixels size of the sidebar_scale
	graph->real_xsize			= graph->xend - graph->xstart;
	graph->ysegment_size		= graph->graph_amp / graph->ysegments; // numbers size of a segment
	graph->xsegment_size		= graph->graph_n / graph->xsegments;
	graph->real_ysegment_size	= graph->real_ysize / (2 * graph->ysegments); // pixels size of a segment
	graph->real_xsegment_size	= graph->real_xsize / graph->xsegments;

	// Setup segment texts

	graph->font = graph_open_font(setup->font_path, setup->font_size);

	graph->plot_color = setup->plot_color;
	graph->fg_color = setup->fg_color;
	graph->bg_color = setup->bg_color;
	
	graph->ysegment_count = graph->ysegments * 2 + 1;
	graph->ysegment_texts = (textpos_t*) malloc(sizeof(*graph->ysegment_texts) * graph->ysegment_count); // textpos_t*

	int ysegments_done = 0;

	for (int i = graph->graph_amp; i >= -graph->graph_amp; i -= graph->ysegment_size) {
		SDL_Surface* surf;
		SDL_Texture* txt;

		#ifndef GRAPHING_NO_CACHING
			segmentsurf_entry_t *ss_buff, *ss_found;

			ss_buff = (segmentsurf_entry_t*) malloc(sizeof(*ss_buff)); // segmentsurf_entry_t*
			ss_buff->key.font = graph->font;
			ss_buff->key.color = graph->fg_color;

			const int maxlen = sizeof(ss_buff->key.label) - 1;
			snprintf(ss_buff->key.label, maxlen + 1, "%*d", maxlen, i);

			HASH_FIND(hh, segmentsurf_cache, &ss_buff->key /* or just &ss_buff */, sizeof(sscache_key_t), ss_found);
			if (!ss_found) {
				// printf(">> generating new surface <<\n");
				ss_buff->surf = TTF_RenderUTF8_Solid(ss_buff->key.font, ss_buff->key.label, ss_buff->key.color);
				
				if (ss_buff->surf) {
					HASH_ADD(hh, segmentsurf_cache, key, sizeof(sscache_key_t), ss_buff);
				} else { /* else crash/set empty label ? */
					printf("render text on surface error: %s\n", TTF_GetError());
				}
				surf = ss_buff->surf;

			} else {
				surf = ss_found->surf;
				free(ss_buff);
			}

		#else
			const int maxlen = 11+1;
			char buff[maxlen];
			snprintf(buff, maxlen + 1, "%*d", maxlen, i);
			surf = TTF_RenderUTF8_Solid(graph->font, buff, graph->fg_color);
		#endif

		txt = SDL_CreateTextureFromSurface(graph->window.renderer, surf);

		// Adjust texture postion based on the size of allocated slot
		int xoffset = graph->xstart - surf->w;
		int yoffset = graph->ystart + ysegments_done * graph->real_ysegment_size - surf->h/2;
		
		if (surf->h < graph->real_ysegment_size)
			yoffset += 0; //(graph->real_ysegment_size - surf->h) / 2;
		
		SDL_Rect rect = {
			(int)(xoffset - 0.10 * graph->xmargin),
			yoffset,
			surf->w,
			surf->h,
		};
		graph->ysegment_texts[ysegments_done] = (textpos_t){rect, txt};

		#ifdef GRAPHING_NO_CACHING
			SDL_FreeSurface(surf);
		#endif

		ysegments_done++;
	}

	#ifdef GRAPHING_USE_MUTEX
	graph->mutex = SDL_CreateMutex();
	#endif

	// Initialize data buffer
	graph->pos = 0;

	graph->points_f = (float*) malloc(sizeof(*graph->points_f) * graph->graph_n); // float*
	for (int i = 0; i < graph->graph_n; i++)
		graph->points_f[i] = 0.0f;
	
	graph->_render_points_f = (SDL_FPoint*) malloc(sizeof(*graph->_render_points_f) * graph->graph_n); // SDL_FPoint*

	const float xscale = (float)graph->real_xsize / graph->graph_n;
	for (int xx = 0; xx < graph->graph_n; xx++)
		graph->_render_points_f[xx] = (SDL_FPoint){graph->xstart + xscale * xx + 1, (float)graph->zeroy};

	// Draw initial sidebar & segment texts
	graph_init_render_(graph);

	// Release setup struct by default
	if (setup->consume)
		free(setup);

	return graph;
}


/* */
void graph_init_render_(graph_t* graph)
{
	SDL_SetRenderDrawColor(graph->window.renderer, graph->bg_color.r, graph->bg_color.g, graph->bg_color.b, GRAPHING_DEFAULT_BG_ALPHA);
	SDL_RenderClear(graph->window.renderer);
	SDL_SetRenderDrawColor(graph->window.renderer, graph->fg_color.r, graph->fg_color.g, graph->fg_color.b, GRAPHING_DEFAULT_FG_ALPHA);
	// Redraw sidebar_scale
	SDL_RenderDrawLines(graph->window.renderer, &graph->sidebar_scale[0], ARR_SIZE(graph->sidebar_scale));

	// Redraw numbers
	for(int i = 0; i < graph->ysegment_count; i++)
		SDL_RenderCopy(graph->window.renderer, graph->ysegment_texts[i].texture, NULL, &(graph->ysegment_texts[i].rect));

	// SDL_RenderPresent(graph->window.renderer);
}


/* */
void graph_destroy(graph_t* graph)
{
	if (graph == NULL)
		return;

	#ifdef GRAPHING_USE_MUTEX
		SDL_DestroyMutex(graph->mutex);
	#endif

	for(int i = 0; i < graph->ysegment_count; i++) 
		SDL_DestroyTexture(graph->ysegment_texts[i].texture);
	free(graph->ysegment_texts);

	close_window(&graph->window);

	#ifdef GRAPHING_NO_CACHING
	if (graph->font) {
		TTF_CloseFont(graph->font);
		graph->font = NULL;
	}
	#endif
	
	free(graph->points_f);
	free(graph->_render_points_f);
	free(graph->window.title);
	free(graph);
}


/* */
void graph_render(graph_t* graph)
{
	// TODO: save color and restore after

	SDL_Rect fillRect = {graph->xstart - 1, graph->ystart - 1, graph->xend, graph->yend};
	SDL_SetRenderDrawColor(graph->window.renderer, graph->bg_color.r, graph->bg_color.g, graph->bg_color.b, GRAPHING_DEFAULT_BG_ALPHA); // white-grayish

	SDL_RenderFillRect(graph->window.renderer, &fillRect);
	// SDL_RenderClear(graph->window.renderer);

	SDL_SetRenderDrawColor(graph->window.renderer, graph->fg_color.r, graph->fg_color.g, graph->fg_color.b, GRAPHING_DEFAULT_FG_ALPHA); // black 
	// Redraw sidebar_scale
	SDL_RenderDrawLines(graph->window.renderer, &graph->sidebar_scale[0], ARR_SIZE(graph->sidebar_scale));

	// Draw points
	const float yscale = (float)graph->real_ysize / (2 * graph->graph_amp);

	int oidx = 0;

	// [pos, last]
	for (int xx = graph->pos; xx < graph->graph_n; xx++,oidx++) {
		float yy = graph->points_f[xx];
		graph->_render_points_f[oidx].y = graph->zeroy - (yy * yscale);
	}
	// [first, pos)
	for (int xx = 0; xx < graph->pos; xx++,oidx++) {
		float yy = graph->points_f[xx];
		graph->_render_points_f[oidx].y = graph->zeroy - (yy * yscale);
	}

	SDL_SetRenderDrawColor(graph->window.renderer, graph->plot_color.r, graph->plot_color.g, graph->plot_color.b, GRAPHING_DEFAULT_PLOT_ALPHA);
	SDL_RenderDrawLinesF(graph->window.renderer, &(graph->_render_points_f)[0], graph->graph_n);
	SDL_RenderPresent(graph->window.renderer);
}


/* */
void graph_addpoint(graph_t* graph, int x, float y)
{
	#ifdef GRAPHING_USE_MUTEX
		SDL_LockMutex(graph->mutex);
	#endif

	graph->points_f[x] = y;

	#ifdef GRAPHING_USE_MUTEX
		SDL_UnlockMutex(graph->mutex);
	#endif
}


/* */
void graph_appendpoint(graph_t* graph, float y)
{
	#ifdef GRAPHING_USE_MUTEX
		SDL_LockMutex(graph->mutex);
	#endif

	graph->points_f[graph->pos] = y; // graph_addpoint(graph, graph->pos, y);
	graph->pos = (graph->pos + 1) % graph->graph_n;
	
	#ifdef GRAPHING_USE_MUTEX
		SDL_UnlockMutex(graph->mutex);
	#endif
}


/* */
int graph_load()
{
	/*
		(*) sdl init fail:			00000001 1
		(*) img load fail:			00001000 1<<3
		(*) ttf load fail:			00010000 1<<4
		(*) mixer open fail:		00100000 1<<5
		(*) net open fail:			01000000 1<<6
	*/
	
	unsigned int error = 0;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		error |= 1;
		return error; // fatal
	}

	// Fonts
	if (TTF_Init() == -1) {
		printf("SDL_TTF cannot initialize! Error: %s\n", TTF_GetError());
		error |= 1<<4;
	}

	/* if(SDLNet_Init() == -1) {
		printf("SDL_net cannot initialize! Error: %s\n", SDLNet_GetError());
		error |= 1<<6;
	} */

	// Hint to fix some window focus issues
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

	// [!] Try to fix flickering
	// // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d");
	// // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d12");
	// // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
	// #ifdef __linux__
	// 	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    // 	SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "2");
	// #endif

	return error;
}


/* */
int create_window(Window* w, char* title, int width, int height, int fps, int xpos, int ypos)
{
	/*
		(*) window creation fail:	00000010 1<<1
		(*) renderer creation fail:	00000100 1<<2
	*/

	unsigned int error = 0;

	w->title = title;
	w->width = width;
	w->height = height;
	if (fps < 1)
		fps = 60;
	w->fps = fps;

	// Create app window
	// int xpos = randint(500,0); // SDL_WINDOWPOS_UNDEFINED;
	// int ypos = randint(500,0); // SDL_WINDOWPOS_UNDEFINED;
	if (xpos == -1) xpos = SDL_WINDOWPOS_UNDEFINED;
	if (ypos == -1) ypos = SDL_WINDOWPOS_UNDEFINED;

	w->window = SDL_CreateWindow(w->title, xpos, ypos, w->width, w->height, SDL_WINDOW_SHOWN);
	if (w->window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		error |= 1<<1;
		return error; // fatal
	}

	// Create renderer for the window
	w->renderer = SDL_CreateRenderer(w->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (w->renderer == NULL) {
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		error |= 1<<2;
		return error; // fatal
	}

	//Init renderer color
	SDL_SetRenderDrawColor(w->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	return error;
}


/* */
void close_window(Window* w)
{
	SDL_DestroyRenderer(w->renderer);
	SDL_DestroyWindow(w->window);
	w->renderer = NULL;
	w->window = NULL;
}


/* */
void graph_shutdown()
{
	#ifndef GRAPHING_NO_CACHING
		// Destroy caches
		font_entry_t *elem_fnt, *tmp_fnt;
		HASH_ITER(hh, /* map name */ g_font_cache, elem_fnt, tmp_fnt) {
			TTF_CloseFont(elem_fnt->font);
			HASH_DEL(g_font_cache, elem_fnt);
			free(elem_fnt);
		}

		segmentsurf_entry_t *elem_ss, *tmp_ss;
		HASH_ITER(hh, /* map name */ segmentsurf_cache, elem_ss, tmp_ss) {
			HASH_DEL(segmentsurf_cache, elem_ss);
			SDL_FreeSurface(elem_ss->surf);
			free(elem_ss);
		}
	#endif

	// Destroy SDL
	// SDLNet_Quit();
	TTF_Quit();
	SDL_Quit();
}

#ifdef __cplusplus
}
#endif

#endif //__GRAPHING_H_
