#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "../utils/shader.h"

class CamV2
{
public:
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 cameraMatrix = glm::mat4(1.0f);

	bool firstClick = true;

	int width;
	int height;

	float speed = 0.1f;
	float sensitivity = 100.0f;

	CamV2(int width, int height, glm::vec3 position)
	{
		this->width = width;
		this->height = height;
		this->Position = position;
	};

	void updateMatrix(float FOVdeg, float nearPlane, float farPlane)
	{
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		view = glm::lookAt(Position, Position + Orientation, Up);
		projection = glm::perspective(glm::radians(FOVdeg), float(width) / height,
			nearPlane, farPlane);

		cameraMatrix = projection * view;
	}

	void Matrix(Shader& shader, const char* uniform)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, uniform),
			1, GL_FALSE, glm::value_ptr(cameraMatrix));
	}

	void Inputs(GLFWwindow* window)
	{
		//WASD
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			Position += speed * Orientation;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			Position += speed * -glm::normalize(glm::cross(Orientation, Up));
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			Position += speed * -Orientation;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			Position += speed * glm::normalize(glm::cross(Orientation, Up));

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			Position += speed * Up;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			Position += speed * -Up;
		}

		//
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			Position += speed * Up;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			Position += speed * -Up;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) 
		{

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstClick)
			{
				glfwSetCursorPos(window, (width / 2), (height / 2));
				firstClick = false;
			}

			double mouseX;
			double mouseY;

			glfwGetCursorPos(window, &mouseX, &mouseY);

			float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
			float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

			glm::vec3 newOrientation = glm::rotate(Orientation,
				glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

			if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				Orientation = newOrientation;
			}

			Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

			glfwSetCursorPos(window, (width / 2), (height / 2));

		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			firstClick = true;

		}

	}

};

#endif