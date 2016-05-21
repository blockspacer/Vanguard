#include <allegro5/allegro.h>
#include <cstdio>

#include "renderer.h"
#include "global_constants.h"

Renderer::Renderer()
{
	// Create a display
    display = al_create_display(WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!display)
    {
		// FIXME: Make the error argument mean anything?
        fprintf(stderr, "Fatal Error: Could not create display\n");
        throw -1;
    }
}

Renderer::~Renderer()
{
	// Cleanup
	al_destroy_display(display);
}

void Renderer::render()
{
	// Draw everything
    al_clear_to_color(al_map_rgb(0,0,0));

    al_flip_display();
}
