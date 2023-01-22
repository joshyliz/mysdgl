#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct 
{
	int width;
	int height;
	unsigned int data;
	unsigned int ID;
} Texture;

