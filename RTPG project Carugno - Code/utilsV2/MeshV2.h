#pragma once
using namespace std;

#include <string>

#include "VAO.h"
#include "EBO.h"
#include "CamV2.h"

class MeshV2
{
public:

	vector<Vertex> vertices;
	vector<GLuint> indices;

    VAO VAO;

    //VBO VBO;
    //EBO EBO;

    
    MeshV2(vector<Vertex>& vertices, vector<GLuint>& indices) //: VBO(vertices), EBO(indices)
    {

        this->vertices = vertices;
        this->indices = indices;

        VAO.Bind();

        VBO VBO(vertices);
        EBO EBO(indices);

        VAO.LinkAttribute(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
        VAO.LinkAttribute(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
        VAO.LinkAttribute(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));

        VAO.Unbind();
        //
        VBO.Unbind();
        EBO.Unbind();
        

    }
    

    void Draw(Shader& shader, CamV2& camera, 
        glm::mat4 matrix = glm::mat4(1.0f),
        glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f))
    {
        shader.Use();
        VAO.Bind();

        glUniform3f(glGetUniformLocation(shader.Program, "camPos"),
            camera.Position.x, camera.Position.y, camera.Position.z);
        camera.Matrix(shader, "camMatrix");

        glm::mat4 trans = glm::mat4(1.0f);
        glm::mat4 rot = glm::mat4(1.0f);
        glm::mat4 sca = glm::mat4(1.0f);

        trans = glm::translate(trans, translation);
        rot = glm::mat4_cast(rotation);
        sca = glm::scale(sca, scale);

        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "translation"), 
            1, GL_FALSE, glm::value_ptr(trans));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "rotation"), 
            1, GL_FALSE, glm::value_ptr(rot));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "scale"), 
            1, GL_FALSE, glm::value_ptr(sca));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 
            1, GL_FALSE, glm::value_ptr(matrix));

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //Updated version
    //V2
    MeshV2(vector<Vertex>& vertices, vector<GLuint>& indices, int dummy)
    {
        this->vertices = vertices;
        this->indices = indices;
    }


    void drawV2(Shader& shader, CamV2& camera,
        glm::mat4 matrix = glm::mat4(1.0f),
        glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f))
    {
        // Create a VAO and bind it
        VAO;
        VAO.Bind();

        // Create a VBO and bind it
        VBO vbo(vertices);
        vbo.Bind();

        // Create an EBO and bind it
        EBO ebo(indices);
        ebo.Bind();

        // Set the vertex attribute pointers for the VBO
        VAO.LinkAttribute(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
        VAO.LinkAttribute(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
        VAO.LinkAttribute(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));

        // Unbind the VAO, VBO, and EBO
        VAO.Unbind();
        vbo.Unbind();
        ebo.Unbind();

        // Use the shader program and bind the VAO
        shader.Use();
        VAO.Bind();

        glUniform3f(glGetUniformLocation(shader.Program, "camPos"),
            camera.Position.x, camera.Position.y, camera.Position.z);
        camera.Matrix(shader, "camMatrix");

        glm::mat4 trans = glm::mat4(1.0f);
        glm::mat4 rot = glm::mat4(1.0f);
        glm::mat4 sca = glm::mat4(1.0f);

        trans = glm::translate(trans, translation);
        rot = glm::mat4_cast(rotation);
        sca = glm::scale(sca, scale);

        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "translation"),
            1, GL_FALSE, glm::value_ptr(trans));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "rotation"),
            1, GL_FALSE, glm::value_ptr(rot));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "scale"),
            1, GL_FALSE, glm::value_ptr(sca));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(matrix));

        // Draw the mesh using the indices stored in the EBO
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Unbind the VAO
        VAO.Unbind();

        // Delete the VAO, VBO, and EBO to clean up resources
        VAO.Delete();
        vbo.Delete();
        ebo.Delete();

    }

};