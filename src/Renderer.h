#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "types.h"
#include <ft2build.h>
#include FT_FREETYPE_H

void StartWindow(const char *title, int width, int height, Uint32 flags);

Texture LoadTexture(const char *filePath);

Font LoadFont(int fontSize, const char *filePath);

void EventChecks();

void BeginDrawing();

void EndDrawing();

void DrawTexture(Rectangle rectangle, Texture texture);

void DrawTextureExtra(Rectangle destRectangle, Rectangle srcRectangle, float angle, Texture texture);

void DrawTexturePos(Vector2 pos);

void DrawFont(Vector2 pos, Font font, const char *text);

double GetDeltaTime();

int GetFps();

bool IsRunning();

void SetWantedFps(int fps);

void QuitWindow();

void QuitLoop();

SDL_Window* GetSDLWindow();


