#include "Renderer.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Window *window = NULL;
static SDL_GLContext context;

unsigned int VBO, VAO, EBO;
unsigned int shaderProgram; 

double deltaTime = 0;
Uint64 NOW;
Uint64 LAST;
int fps = 0;
int wantedFps = 60;


float vertices[] = { 
	// pos      // tex
	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 

	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 0.0f
};

unsigned int indices[] = {  
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
};

mat4 projection = GLM_MAT4_IDENTITY_INIT;


void StartWindow(const char *title, int width, int height, Uint32 flags)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized\nError Code: %s\n", SDL_GetError());
	}
	else printf("SDL successfully started!\n");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


	window = SDL_CreateWindow(title, 
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 
			width, 
			height, 
			flags | SDL_WINDOW_OPENGL);

	context = SDL_GL_CreateContext(window);

	gladLoadGLLoader(SDL_GL_GetProcAddress);

	glViewport(0, 0, width, height);

	
	const char *vertexShaderSource = readFile("vert.vs");
	const char *fragmentShaderSource = readFile("frag.fs");

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

	//VBO VAO EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
	glUseProgram(shaderProgram);
	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, projection);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (float*)projection);

	NOW = SDL_GetPerformanceCounter();
	LAST = 0;
}


Texture LoadTexture(const char *filePath)
{
	static int textureNum = 0;

	GLuint TextureID = 0;
	 
	SDL_Surface *Surface = IMG_Load(filePath);

	if(Surface == NULL)
	{
		printf("Failed to load texture: %s\n", filePath);
		return (Texture){0, 0, 0, 0};
	}

	//Turn Off Later
	flip_surface(Surface);
	 
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	 
	int Mode = GL_RGB;
	 
	if(Surface->format->BytesPerPixel == 4) 
	{
		Mode = GL_RGBA;
	}
	 
	glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, 
			GL_UNSIGNED_BYTE, Surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	 
	Texture texture = {Surface->w, Surface->h, textureNum, TextureID};

	SDL_FreeSurface(Surface);

	textureNum++;

	return texture;
}

void EventChecks()
{
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();
	deltaTime = (double)((NOW - LAST) * 1000 / (double) SDL_GetPerformanceFrequency());

	if(deltaTime >! 1000/(double)fps)
	{
		if(wantedFps != 0)
			SDL_Delay(1000 / wantedFps);
	}

	fps = 1000 / deltaTime; 

	int w = 0;
	int h = 0;
	SDL_GetWindowSize(window, &w, &h);
	glViewport(0, 0, w, h);

	SDL_Event event;
	SDL_PollEvent(&event);

	if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
	{
		exit(0);	
	}

	if(event.type == SDL_QUIT)
		exit(0);
}

void BeginDrawing()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void EndDrawing()
{
	SDL_GL_SwapWindow(window);
}

void DrawTexture(Texture texture)
{
	glUseProgram(shaderProgram);
	glActiveTexture(GL_TEXTURE0 + texture.data);
	glBindTexture(GL_TEXTURE_2D, texture.ID);


	vec2 size = {100, 100};

	
	mat4 model = GLM_MAT4_IDENTITY_INIT;
	glm_translate(model, (vec3){0, 0, 0});
	glm_translate(model, (vec3){0.5f * size[0], 0.5f * size[1], 0.0f});
	//glm_rotate(model, glm_rad(45.0f), (vec3){0.0f, 0.0f, 1.0f});
	glm_translate(model, (vec3){-0.5f * size[0], -0.5f * size[1], 0.0f});
	glm_scale(model, (vec3){size[0], size[1], 1.0f});

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (float*)model);
	glUniform3fv(glGetUniformLocation(shaderProgram, "spriteColor"), 1, (vec3){1, 1, 1});


	glUniform1i(glGetUniformLocation(shaderProgram, "image"), texture.data);


	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

double GetDeltaTime()
{
	return deltaTime;
}

void QuitWindow()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

SDL_Window* GetSDLWindow()
{
	return window;
}

