#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cglm/cglm.h>

#define VECT2_ZERO (Vector2){0, 0}
#define RECT_ZERO (Rectangle){0, 0, 0, 0}

//Draw types
typedef struct 
{
	unsigned int width;
	unsigned int height;
	unsigned int data;
	unsigned int ID;
} Texture;

typedef struct 
{
	unsigned int TextureID;
	ivec2 Size;
	ivec2 Bearing;
	unsigned int Advance;
	unsigned int data;
} Character;

typedef struct
{
	Character *characters;
	int Max;
} Font;

//Math types
typedef struct
{
	float x;
	float y;
	float w;
	float h;
} Rectangle;

typedef struct
{
	float x;
	float y;
} Vector2;
