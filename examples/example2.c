/*
 *	Example2.c
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

#define RANDINT(mx, mn) ((int)(((float)rand()/RAND_MAX) * ((mx)-(mn)) + (mn)))
#define INC(x, mx) (x) = ((x) + 1) % (mx)

int main(int argc, char* argv[])
{
	srand(time(NULL));

	// Inits SDL
	graph_load();

	// Configure your new graph...
	graph_setup_t* setup = graph_blank_setup();
	setup->title 		= "Still a kool title";
	setup->width 		= 640;
	setup->height 		= 480;
	setup->graph_amp 	= 1000; // y-values are in range [-400, 400]
	setup->graph_n 		= 500; // x-values are in range [0, 100]
	setup->ysegments 	= 5;

	setup->consume      = 0; // we are going to use it for 2 windows

	// 

	// ...and make the window
	setup->xpos = 50;
	setup->ypos = 50;
	graph_t* graph1 = graph_create(setup);
	setup->xpos += 640;
	setup->ypos += 480;
	graph_t* graph2 = graph_create(setup);

	free(setup); // we have to do this manually because we set .consume to 0!

	// Create some data buffers
	float databuf[] = {0.f};
	int buff_size = ARR_SIZE(databuf); // (defined in .h)
	int idx = 0;

	float last_graph2_point = 0.f; // We are going to copy this point if new one doesn't satisfy condition

	// Proceed with a usual SDL loop
	int running = 1;
	int paused = 0;
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

						case SDLK_p:
							paused = !paused;
							break;
					}
			}
		}

		if (!paused) {
			databuf[idx] = RANDINT(400, -400);      // Pretend this data comes from elsewhere (stdin/other thread)

			// We are going to plot all points to graph1...
			graph_appendpoint(graph1, databuf[idx]); // Add new value to the end of the graph

			// ...but only those greater than abs(300) on graph2; otherwise append last valid point
			// This is going to make the graph look interestingly rectangular...
			if (databuf[idx] > 300 || databuf[idx] < -300) {
				graph_appendpoint(graph2, databuf[idx]);
				last_graph2_point = databuf[idx];
			} else {
				graph_appendpoint(graph2, last_graph2_point);
			}
			
			idx = (idx + 1) % buff_size;            // Increment index counter
			                                        //   (or make the points some sort of queue)
		}

		// Don't forget to render both graphs...
		graph_render(graph1);
		graph_render(graph2);
	}

	// ...and free both graphs at the end
	graph_destroy(graph1);
	graph_destroy(graph2);
	
	// Destroys SDL & caches
	graph_shutdown();
	return 0;
}
