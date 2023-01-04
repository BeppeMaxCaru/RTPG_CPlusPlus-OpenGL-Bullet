#version 410 core

// Outputs colors in RGBA
out vec4 FragColor;

// Imports the current position from the Vertex Shader
in vec3 crntPos;
// Imports the normal from the Vertex Shader
in vec3 Normal;
// Imports the color from the Vertex Shader
in vec3 color;

// Gets the position of the camera from the main function
uniform vec3 camPos;

void main()
{
	FragColor = vec4(color, 1.0f);
}
