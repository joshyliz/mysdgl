#include "Renderer.h"
#include "util.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "write.h"

static SDL_Window *window = NULL;
static SDL_GLContext context;

unsigned int VBO, VAO;
unsigned int shaderProgram; 

unsigned int textVBO;
unsigned int textVAO;

double deltaTime = 0;
Uint64 NOW;
Uint64 LAST;
int fps = 0;
int wantedFps = 60;

bool isRunning;


//Shader var locations
GLint imageLoc; 
GLint colorLoc;
GLint textRenderLoc;
GLint projectionLoc;
GLint modelLoc;
GLint srcRectangleLoc;


float vertices[] = { 
	// pos      // tex
	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 

	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 0.0f
};


mat4 projection = GLM_MAT4_IDENTITY_INIT;

static unsigned int textureNum = 0;

void StartWindow(const char *title, int width, int height, Uint32 flags)
{
	//Init SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized\nError Code: %s\n", SDL_GetError());
		isRunning = false;
	}
	else 
	{
		printf("SDL successfully started!\n");
		isRunning = true;
	}

	//Link SDL to opengl
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	//Make SDL Opengl Window
	window = SDL_CreateWindow(title, 
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 
			width, 
			height, 
			flags | SDL_WINDOW_OPENGL);

	//Make Opengl Context
	context = SDL_GL_CreateContext(window);
	
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	//Set the viewport
	glViewport(0, 0, width, height);

	//Shaders source code	
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

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	//Text Rendering Buffers
	glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
	
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	//Init Variables
	glUseProgram(shaderProgram);
	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//Delta time initialize
	NOW = SDL_GetPerformanceCounter();
	LAST = 0;

	//Enables Transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//For text rendering
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//Shader locations
	imageLoc = glGetUniformLocation(shaderProgram, "image"); 
	colorLoc = glGetUniformLocation(shaderProgram, "spriteColor"); 
	textRenderLoc = glGetUniformLocation(shaderProgram, "textRender"); 
	projectionLoc = glGetUniformLocation(shaderProgram, "projection"); 
	modelLoc = glGetUniformLocation(shaderProgram, "model"); 
	srcRectangleLoc = glGetUniformLocation(shaderProgram, "srcRectangle");

	glm_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f, projection);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (float*)projection);
}

Texture LoadTexture(const char *filePath)
{
	GLuint TextureID = 0;
	 
	SDL_Surface *Surface = IMG_Load(filePath);

	if(Surface == NULL)
	{
		printf("Failed to load texture: %s\n", filePath);
		return (Texture){0, 0, 0, 0};
	}

	//flip_surface(Surface); If you want to load image fliped
	 
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

	glBindTexture(GL_TEXTURE_2D, TextureID);

	textureNum++;

	return texture;
}

Font LoadFont(int fontSize, const char *filePath)
{
	Font font;
	font.characters = calloc(128, sizeof(Character));
	
	FT_Library ft;
	if(FT_Init_FreeType(&ft))
	{
		printf("Could not initialize freetype library\n");
	}

	FT_Face face;
	if(FT_New_Face(ft, filePath, 0, &face))
	{
		printf("Failed to load font\n");
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);

	int maxSize = 0;

	for(unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			printf("ERROR::FREETYTPE: Failed to load Glyph\n");
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);


		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture, 
			{face->glyph->bitmap.width, face->glyph->bitmap.rows},
			{face->glyph->bitmap_left, face->glyph->bitmap_top},
			(unsigned int)face->glyph->advance.x,
			textureNum,
		};
		
		if(character.Size[1] > maxSize)
		{
			maxSize = character.Size[1];
		}

		textureNum++;

		font.characters[c] = character;
	}

	font.Max = maxSize;
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return font;
}

void EventChecks()
{
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();
	deltaTime = (double)((NOW - LAST) * 1000 / (double) SDL_GetPerformanceFrequency());

	SDL_Event event;
	SDL_PollEvent(&event);

	if(event.type == SDL_QUIT)
		isRunning = false;

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
}

void BeginDrawing()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
}

void EndDrawing()
{
	SDL_GL_SwapWindow(window);
}

void DrawTexture(Rectangle rectangle, Texture texture)
{
	vec2 size = {rectangle.w, rectangle.h};

	mat4 model = GLM_MAT4_IDENTITY_INIT;

	glm_translate(model, (vec3){rectangle.x, rectangle.y, 0});
	glm_translate(model, (vec3){0.5f * size[0], 0.5f * size[1], 0.0f});
	glm_rotate(model, glm_rad(0), (vec3){0.0f, 0.0f, 1.0f});
	glm_translate(model, (vec3){-0.5f * size[0], -0.5f * size[1], 0.0f});
	glm_scale(model, (vec3){size[0], size[1], 1.0f});

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);
	glUniform3fv(colorLoc, 1, (vec3){1, 1, 1});
	glUniform1i(imageLoc, texture.data);
	glUniform1i(textRenderLoc, false);

	glActiveTexture(GL_TEXTURE0 + texture.data);
	glBindTexture(GL_TEXTURE_2D, texture.ID);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void DrawTextureExtra(Rectangle destRectangle, Rectangle srcRectangle, float angle, Texture texture)
{
	vec2 size = {destRectangle.w, destRectangle.h};

	mat4 model = GLM_MAT4_IDENTITY_INIT;

	glm_translate(model, (vec3){destRectangle.x, destRectangle.y, 0});
	glm_translate(model, (vec3){0.5f * size[0], 0.5f * size[1], 0.0f});
	glm_rotate(model, glm_rad(angle), (vec3){0.0f, 0.0f, 1.0f});
	glm_translate(model, (vec3){-0.5f * size[0], -0.5f * size[1], 0.0f});
	glm_scale(model, (vec3){size[0], size[1], 1.0f});

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);
	glUniform3fv(colorLoc, 1, (vec3){1, 1, 1});
	glUniform1i(imageLoc, texture.data);
	glUniform1i(textRenderLoc, false);

	
	glUniform4fv(srcRectangleLoc, 1, (vec4){srcRectangle.x, srcRectangle.y, texture.width / srcRectangle.w, texture.height / srcRectangle.h});

	glActiveTexture(GL_TEXTURE0 + texture.data);
	glBindTexture(GL_TEXTURE_2D, texture.ID);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void DrawFont(Vector2 pos, Font font, const char *text)
{
	size_t length = strlen(text);

	float x = pos.y; 
	float y = pos.x + font.Max;
	float scale = 1;

	mat4 model = GLM_MAT4_IDENTITY_INIT;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);
	glUniform3fv(colorLoc, 1, (vec3){1, 1, 1});
	glUniform1i(textRenderLoc, true);

	for (unsigned char c = 0; c != length; c++)
	{
		char t = text[c];

		Character ch = font.characters[t];

		float w = ch.Size[0] * scale;
		float h = ch.Size[1] * scale;

		float xpos = x + ch.Bearing[0] * scale;
		float ypos = -y - (ch.Size[1] - ch.Bearing[1]) * scale;
		//float ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

		// update VBO for each character
		float textVertices[6][4] = {
			{ xpos,     -(ypos + h),   0.0f, 0.0f },            
			{ xpos,     -ypos,       0.0f, 1.0f },
			{ (xpos + w), -ypos,       1.0f, 1.0f },

			{ xpos,     -(ypos + h),   0.0f, 0.0f },
			{ (xpos + w), -ypos,       1.0f, 1.0f },
			{ (xpos + w), -(ypos + h),   1.0f, 0.0f }           
		};

		mat4 model = GLM_MAT4_IDENTITY_INIT;

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);
		glUniform1i(imageLoc, ch.data);

		// render glyph texture over quad
		glActiveTexture(GL_TEXTURE0 + ch.data);
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBindVertexArray(textVAO);
		
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textVertices), textVertices); 
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
}

double GetDeltaTime()
{
	return deltaTime;
}

int GetFps()
{
	return fps;
}

bool IsRunning()
{
	return isRunning; 
}

void SetWantedFps(int fps)
{
	wantedFps = fps;
}

void QuitWindow()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void QuitLoop()
{
	isRunning = false;
}

SDL_Window* GetSDLWindow()
{
	return window;
}


