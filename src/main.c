#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "util.h"
#include "Renderer.h"


int main(int argc, char* argv[])
{
	StartWindow("GameThing", 1280, 720, SDL_WINDOW_RESIZABLE);

	Texture wood = LoadTexture("container.jpg");
	Texture face = LoadTexture("face.png");


	while (true) 
	{
		EventChecks();


		BeginDrawing();


		DrawTexture(wood);


		EndDrawing();
	}

	QuitWindow();

	return 0;
}
