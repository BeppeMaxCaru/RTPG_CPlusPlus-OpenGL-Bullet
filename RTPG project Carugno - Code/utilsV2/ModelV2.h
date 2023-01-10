#pragma once
using namespace std;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftBody.h>

#include "../utilsV2/MeshV2.h"


class ModelV2
{
public:

	//Original meshes
	vector<MeshV2> meshes;

	//Vector to combine the meshes into a unique big mesh
	//Later those two vectors can be used to create rigid bodies or soft bodies
	vector<btVector3> vertices;
	vector<GLuint> indices;

	ModelV2(const string& path)
	{
		//Load model from .obj file
		loadModel(path);

		////////////////////////////////////////////////////////
		//Merge the meshes into one unique mesh to improve performance at runtime
		//Doing it here exactly when loading the model saves computing time later
		//Complexity O(n^2)
		mergeMeshes(meshes, &vertices, &indices);
		//Complexity O(n)
		//mergeMeshes2(meshes, &vertices, &indices);
	}

private:

	//Model loading using recursion
	void loadModel(string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_FixInfacingNormals | aiProcess_FindDegenerates | aiProcess_FindInstances);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}

		processNode(scene->mRootNode, scene);

	}

	void processNode(aiNode* node, const aiScene* scene) {
		//Process all the node's meshes (if any)
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.emplace_back(processMesh(mesh));
		}
		//Then do the same for each of its children
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}
	
	MeshV2 processMesh(aiMesh* mesh) {
		vector<Vertex> vertices;
		vector<GLuint> indices;

		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};

			glm::vec3 vector;
			//Vertices coordinates
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			//Normals
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;


			//vertex.Color = glm::vec3(0.0f, 1.0f, 0.5f);

			vertices.emplace_back(vertex);
		}

		//For each face of the mesh, we retrieve the indices of its vertices , and we store them in a vector data structure
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.emplace_back(face.mIndices[j]);
		}

		//We return an instance of the Mesh class created using the vertices and faces data structures we have created above.
		return MeshV2(vertices, indices);

	}

	///////////////////////////////////////////////////////////////
	//Function to merge meshes into a unique one
	//Complexity is O(n^2)
	void mergeMeshes(vector<MeshV2> meshes, vector<btVector3>* verts, vector<GLuint>* indxs)
	{

		//For a cube 8 vertices and 36 indices!
		vector<btVector3> vertices;
		vector<GLuint> indices;

		//Mapping between the indices of the old vertices vector and the indices of the new vertices vector
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			// Loop through the indices of the current mesh
			for (unsigned int j = 0; j < meshes[i].indices.size(); j++)
			{
				// Find the index of the vertex in the combined vertices array
				int index = -1;
				for (int k = 0; k < vertices.size(); k++)
				{
					if (vertices[k].x() == meshes[i].vertices[meshes[i].indices[j]].Position.x &&
						vertices[k].y() == meshes[i].vertices[meshes[i].indices[j]].Position.y &&
						vertices[k].z() == meshes[i].vertices[meshes[i].indices[j]].Position.z
						)
					{
						index = k;
						break;
					}
				}
				//If the vertex was not found in the combined vertices array, add it
				if (index == -1)
				{
					btVector3 vertex(
						meshes[i].vertices[meshes[i].indices[j]].Position.x,
						meshes[i].vertices[meshes[i].indices[j]].Position.y,
						meshes[i].vertices[meshes[i].indices[j]].Position.z
					);
					vertices.push_back(vertex);
					index = vertices.size() - 1;
				}

				//Add the index of the vertex to the combined indices array
				indices.push_back(index);

				cout << "Merging index " << j << " of mesh " << i << endl;

			}
		}

		*verts = vertices;
		*indxs = indices;

		cout << vertices.size() << "merged Vertices" << endl;
		cout << indices.size() << "merged Indices" << endl;

	}

	/////////////////////////////////////////////////////////////////////
	//To achieve complexity O(n) an unorderep map must be used!

};
