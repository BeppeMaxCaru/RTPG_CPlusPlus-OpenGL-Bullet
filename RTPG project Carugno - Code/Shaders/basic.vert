#version 410 core

//Positions/Coordinates
layout (location = 0) in vec3 aPos;
//Normals
layout (location = 1) in vec3 aNormal;
// Colors
layout (location = 2) in vec3 aColor;

out vec3 crntPos;
out vec3 Normal;
// Outputs the color for the Fragment Shader
out vec3 color;


//NEW
uniform mat4 camMatrix;
//
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;

void main()
{

	// calculates current position
	crntPos = vec3(model * translation * rotation * scale * vec4(aPos, 1.0f));
	// Assigns the normal from the Vertex Data to "Normal"
	Normal = aNormal;
	// Assigns the colors from the Vertex Data to "color"
	color = aColor;

	// Outputs the positions/coordinates of all vertices
	gl_Position = camMatrix * vec4(crntPos, 1.0);

}