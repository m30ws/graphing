/*
 *	Example1.c
 *	
 *	Notes:
 *	Unfortunately, SDL event loop may block while moving window which
 *	means you might have to separate things into threads.
 *	
 */

// #define SDL_MAIN_HANDLED // Define here or in compile flags (mingw)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <graphing.h>

#define RAND_LO -500
#define RAND_HI +500
#define RANDVAL() ((int)(((float)rand()/RAND_MAX) * ((RAND_HI)-(RAND_LO)) + (RAND_LO)))
#define INC(x, mx) (x) = ((x) + 1) % (mx)

int main(int argc, char* argv[])
{
	srand(time(NULL));

	// Inits SDL
	graph_load();

	// Configure your new graph...
	graph_setup_t* setup = graph_blank_setup();
	setup->width = 640;
	setup->height = 480;
	setup->title = "My title";

	// ...and make the window
	graph_t* graph = graph_create(setup);

	// Create some data buffers
	float databuf[] = {0.f};
	int buff_size = ARR_SIZE(databuf); // (defined in .h)
	int idx = 0;

	// Proceed with a usual SDL loop
	int running = 1;
	SDL_Event e;
	while (running) {
		while (SDL_PollEvent(&e) != 0) {
			switch(e.type) {

				case SDL_QUIT:
					running = 0;
					break;
				
				case SDL_KEYDOWN:
					switch (e.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = 0;
							break;
					}
			}
		}

		databuf[idx] = RANDVAL();               // Pretend this data comes from elsewhere (stdin/other thread)
		graph_appendpoint(graph, databuf[idx]); // Add new value to the end of the graph
		idx = (idx + 1) % buff_size;            // Increment index counter
		                                        //   (or make the points some sort of queue)

		// Display
		graph_render(graph);
	}

	// Free mem
	graph_destroy(graph);
	
	graph_shutdown();
	return 0;
}