#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void flip_surface(SDL_Surface* surface);

char* readFile(char* filename);

