#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform bool textRender;

uniform vec4 srcRectangle;

void main()
{    
	if(textRender == false)		
	{
		vec2 source = vec2((TexCoords.x + srcRectangle.x) / srcRectangle.z, (TexCoords.y + srcRectangle.y) / srcRectangle.w);
		color = vec4(spriteColor, 1.0) * texture(image, source);
	}
	else 
	{
		vec4  sampled = vec4(1.0, 1.0, 1.0, texture(image, TexCoords).r);
		color = vec4(spriteColor, 1.0) * sampled;
	}
}  
