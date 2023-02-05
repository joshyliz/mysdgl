#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <string.h>
#include "util.h"
#include "Renderer.h"

int main(int argc, char* argv[])
{
	StartWindow("GameThing", 1280, 720, SDL_WINDOW_RESIZABLE);

	Texture wood = LoadTexture("container.jpg");
	Texture face = LoadTexture("face.png");

	Font font = LoadFont(12, "arial.ttf");

	char text[25];

	int a = 0;

	float timer = 0;

	SetWantedFps(60);

	while (IsRunning()) 
	{
		EventChecks();

		BeginDrawing();

		const Uint8 *key = SDL_GetKeyboardState(NULL);

		if(key[SDL_SCANCODE_ESCAPE])
			exit(0);


		EndDrawing();
	}

	QuitWindow();

	return 0;
}
