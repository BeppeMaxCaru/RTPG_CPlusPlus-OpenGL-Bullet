#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Color;

	bool operator==(const Vertex& vertex)
	{
		return (
			Position.x == vertex.Position.x &&
			Position.y == vertex.Position.y &&
			Position.z == vertex.Position.z &&
			Normal.x == vertex.Normal.x &&
			Normal.y == vertex.Normal.y &&
			Normal.z == vertex.Normal.z &&
			Color.x == vertex.Color.x &&
			Color.y == vertex.Color.y &&
			Color.z == vertex.Color.z
			);
	}
};

class VBO
{
public:
	GLuint ID;

	VBO(vector<Vertex>& vertices)
	{
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
			vertices.data(), GL_STATIC_DRAW);
	}

	void Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, ID);
	};

	void Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	};

	void Delete()
	{
		glDeleteBuffers(1, &ID);
	};

};

#endif