#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "types.h"

void StartWindow(const char *title, int width, int height, Uint32 flags);

Texture LoadTexture(const char *filePath);

void EventChecks();

void BeginDrawing();

void EndDrawing();

void DrawTexture(Texture texture);

double GetDeltaTime();

void QuitWindow();

SDL_Window* GetSDLWindow();


