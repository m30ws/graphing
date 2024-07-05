/*
 *	Copyright (c) 2023-2024 Fran Tomljenović
 *
 *	Available defines:
 *	==================
 *	#define GRAPHING_NO_AUTO_WIN_UPDATES
 *	- by default whenever any one window is clicked, shown or (mini|maxi)mized
 *	  all will be re-focused which should give better experience but has worse
 *	  performance with a large amount of windows
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <vector>
#include <iostream> // getline(cin) for data
#include <string>

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
// #include "SDL2/SDL_mutex.h"
// #include "SDL2/SDL_net.h"

/*#ifdef __linux
#define GRAPHING_USE_SYSTEM_SDL
#endif*/

#define GRAPHING_NO_AUTO_WIN_UPDATES
#include "graphing.h"

#include "cJSON/cJSON.h"

/* #ifdef _WIN32
	#include <windows.h>
	#define KillThread(t) (TerminateThread(SDL_GetThreadID(t), 0))
#endif
#ifdef __linux
	#include <pthread.h>
	#define KillThread(t) (pthread_kill(SDL_GetThreadID(t), 0))
#endif */

#ifndef ARR_SIZE
#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

// 180,135,6
// #define WIDTH 180
// #define HEIGHT 135
// #define N_GRAPHS_IN_COLUMN 6
#define WIDTH 300
#define HEIGHT 225
#define N_GRAPHS_IN_COLUMN 4

#define DELIM ", "

typedef struct {
	std::vector<graph_t*> * graph_list;
	// SDL_mutex* mutex;
} graph_control_t;

uint8_t g_running = 1;
uint8_t g_render_thread_done = 0;
uint8_t g_input_thread_done = 0;
uint8_t g_render_paused = 0;


/* */
std::vector<std::string> string_split(std::string& ss, const char* delim)
{
	size_t pos_start = 0;
	size_t pos_end;
	size_t delim_len = strlen(delim);
	std::string buff;
	std::vector<std::string> res;

	while ((pos_end = ss.find(delim, pos_start)) != std::string::npos) {
		buff = ss.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(buff);
	}

	buff = ss.substr(pos_start);
	if (buff.length() > 0)
		res.push_back(buff);
	return res;
}


/* */
int threaded_input(void* data)
{
	std::vector<graph_t*>* graph_list = ((graph_control_t*) data)->graph_list;
	// SDL_mutex* mainmutex = ((graph_control_t*) data)->mutex;

	std::string buff;
	std::vector<std::string> separated;
	float yvalf;

	while (g_running) {
		std::getline(std::cin, buff); // Kinda blocks :/

		if (!g_running) // repeat check because getline blocks
			break;

		if (buff == "exit") {
			g_running = 0;
			break;
		}

		separated = string_split(buff, DELIM);
		if (separated.size() < graph_list->size()) {
			continue; // Silently ignore packet
		}

		for (size_t i = 0; i < graph_list->size(); i++) {
			yvalf = strtof(separated[i].c_str(), NULL);
			graph_appendpoint(graph_list->at(i), yvalf);
		}
	}
	g_input_thread_done = 1;
	return 0;
}


/* */
int threaded_render(void* glist)
{
	auto graph_list = (std::vector<graph_t*>*) glist;

	uint64_t startticks;
	uint64_t nowticks;

	const int framedelay = (float)1000 / 60;

	while (g_running) {
		startticks = SDL_GetTicks64();
	
		if (! g_render_paused) {
			for (graph_t* gr : *graph_list) {
				graph_render(gr);
			}
		}

		nowticks = SDL_GetTicks64();
		if (nowticks - startticks < framedelay) {
			SDL_Delay(framedelay - (nowticks-startticks));
		}
	}

	g_render_thread_done = 1;
	return 0;
}


/* */
void mainloop_quit()
{
	SDL_Event _ev;
	_ev.type = SDL_QUIT;
	SDL_PushEvent(&_ev);
}


int main(int argc, char* argv[])
{
/*
	Info about graph_setup_t.

	// When modifying fewer properties can use:
	graph_setup_t* setup = graph_blank_setup();
	setup->title = "V3ry kool title";
	fprintf(stdout, "Default path :: |%s|\n", setup->font_path);

	// If .consume was set to 0 requires manual free(setup);

	// Careful to also free() any strings or memory that was manually allocated
	// because only the struct itself will be automatically freed with .consume
*/

	int err;
	if ((err = graph_load()) != 0) {
		printf("SDL load error! (%d)\n", err);
		return err;
	}

	// FILE* fp;
	// if ((fp = fopen("cfg.json", "r")) == NULL) {
	// 	printf("ERROR OPENING JSON FILE");

	// } else {
	// 	char buff[1024+1];
	// 	int nread = fread(buff, 1, sizeof(buff), fp);
	// 	//buff[nread] = '\0';
	// 	printf("nread: %d\n", nread);

	// 	cJSON* json = cJSON_Parse(buff);
	// 	if (json != NULL) {
	// 		char* pretty = cJSON_Print(json);
	// 		printf("JSON:\n%s\n", pretty);
	// 		cJSON_free(pretty);
	// 	}
	// 	cJSON_Delete(json);
	// 	fclose(fp);
	// }

	// Pickup graph params from cmd
	int ngraphs = 1;
	if (argc > 1) {
		ngraphs = atoi(argv[1]);
		if (ngraphs < 1) {
			printf("Cannot create < 1 windows >:( , defaulting to 1!\n");
			ngraphs = 1;
		}
	}

	std::vector<graph_t*>	graph_list;
	graph_control_t ctl = {&graph_list};
	// SDL_mutex* mainmutex = SDL_CreateMutex();
	// graph_control_t ctl = {&graph_list, mainmutex};

	SDL_DisplayMode scr;
	SDL_GetCurrentDisplayMode(0, &scr);
	int ychunk = HEIGHT; // scr.h
	int xchunk = WIDTH; // scr.w
	int title_offset = 26;
	
	char titlebuff[32+1];
	for (int i = 0; i < ngraphs; i++) {
		graph_setup_t* setup = graph_blank_setup();

		// Set window title
		sprintf(titlebuff, "Graph :: %d", i+1);
		setup->title = titlebuff;
		setup->width = WIDTH;
		setup->height = HEIGHT;

		// Calc and set x and y on screen where window is places
		setup->xpos = (((i / N_GRAPHS_IN_COLUMN) * xchunk)) % scr.w;
		setup->ypos = (((i % N_GRAPHS_IN_COLUMN) * (ychunk + title_offset)) + title_offset) % scr.h;

		// Lets change up some colors dunno
		if (i % 2)
			setup->fg_color = (SDL_Color){0x00,0xDB,0xD2};
		else
			setup->fg_color = (SDL_Color){0xD0,0x00,0xD0};
		
		graph_t* gr = graph_create(setup);
		graph_list.push_back(gr);
	}

	SDL_Thread* thr_inp = SDL_CreateThread(threaded_input, "input thread", (void*) &ctl);
	SDL_Thread* thr_ren = SDL_CreateThread(threaded_render, "render thread", (void*) &graph_list);

	//// (Main loop)
	SDL_Event e;
	while (g_running) {
		while (SDL_WaitEvent(&e) != 0) {
			switch(e.type) {

				case SDL_QUIT:
					g_running = 0;
					// KillThread(thr);
					goto afterevents;
					break; // 

				case SDL_KEYDOWN:
					switch (e.key.keysym.sym) {
						// Esc
						case SDLK_ESCAPE:
							mainloop_quit();
							break;
						case SDLK_p:
							g_render_paused = (g_render_paused + 1) % 2;
							break;
					}
					break;

				case SDL_WINDOWEVENT:
					if (!g_running)
						break;

					switch (e.window.event) {

						// User click on X button
						case SDL_WINDOWEVENT_CLOSE:
							mainloop_quit();
							break;

						case SDL_WINDOWEVENT_MINIMIZED:
							for (graph_t* gr : graph_list) {
								SDL_MinimizeWindow(gr->window.window);
							}
							SDL_FlushEvent(SDL_WINDOWEVENT);
							break;
						
						case SDL_WINDOWEVENT_RESTORED:
							for (graph_t* gr : graph_list) {
								SDL_RestoreWindow(gr->window.window);
							}
							SDL_FlushEvent(SDL_WINDOWEVENT);
							break;

					#ifndef GRAPHING_NO_AUTO_WIN_UPDATES

						case SDL_WINDOWEVENT_FOCUS_GAINED:
							int event_to_check = -1;
							int ev_n = 5;
							SDL_Event evnts[ev_n];

							int peeped = SDL_PeepEvents(&evnts[0], ARR_SIZE(evnts), SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
							for (int ii = 0; ii < peeped; ii++) {
								if (event_to_check < 0 && evnts[ii].type == SDL_WINDOWEVENT) {
									event_to_check = evnts[ii].window.event;
								}
							} 

							// set it to one of the ignored events
							if (event_to_check < 0)
								event_to_check = SDL_WINDOWEVENT_MOVED;

							switch (event_to_check) {
								case SDL_WINDOWEVENT_CLOSE:
								case SDL_WINDOWEVENT_RESTORED:
								case SDL_WINDOWEVENT_MINIMIZED:
								// case SDL_WINDOWEVENT_EXPOSED:
								case SDL_WINDOWEVENT_MAXIMIZED:
								case SDL_WINDOWEVENT_FOCUS_LOST:
								case SDL_WINDOWEVENT_MOVED:
								case SDL_WINDOWEVENT_RESIZED:
								case SDL_WINDOWEVENT_SIZE_CHANGED:
									printf("-- ignoring event; (next: %x)\n", evnts[0].window.event);
									break;

								default:
									// printf(" -!- Restoring all windows into focus %d\n", evnts[0].window.event);

									SDL_Window* getwin = SDL_GetWindowFromID(e.window.windowID);
									for (graph_t* gr : graph_list) {
										SDL_RaiseWindow(gr->window.window);
									}
									SDL_RaiseWindow(getwin);
									SDL_FlushEvent(SDL_WINDOWEVENT);

									break;
							}
							break;

					#endif
					}
					break;
			}
		}

	afterevents:
		(void)0;
	}

	// wait for threads
	// With slow inputs input_thread will block because of getline and the program will exit later/never
	while (!g_render_thread_done) // && !g_input_thread_done)
		;

	// #ifndef NO_MUTEX
	// SDL_DestroyMutex(mainmutex);
	// #endif

	// SDL_WaitThread(thr_inp, NULL);
	// SDL_WaitThread(thr_ren, NULL);

	for (graph_t* gr : graph_list)
		graph_destroy(gr);

	graph_shutdown();
	printf("Exited gracefully.\n");	
	return 0;
}
