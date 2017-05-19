#ifndef _CAVE_H
#define _CAVE_H

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cave
{
public:
	Cave();
	~Cave();

	glm::mat4 toWorld;

	void initialize();
	void draw(GLuint, glm::mat4, glm::mat4, GLuint left, GLuint right, GLuint bottom);
	unsigned char* loadPPM(const char*, int&, int&);

	// Cubemap
	void loadCubemap();
	void useCubemap(int eyeIdx);
	glm::vec3 direction = glm::vec3(-0.0459845f, 0.0925645f, 0.994644f);

	// These variables are needed for the shader program
	// GLuint VBO, VAO, uv_ID;

	GLuint lVBO, lVAO, luv_ID;
	GLuint rVBO, rVAO, ruv_ID;
	GLuint bVBO, bVAO, buv_ID;

	GLuint uProjection, uModel, uView, texture_ID_left, texture_ID_right, texture_ID_self, texture_ID, curTextureID;

	/*
	const GLfloat vertices[54] = {
		-2.0,  2.0, -2.0,
		2.0,  2.0, -2.0,
		2.0, -2.0, -2.0,
		2.0, -2.0, -2.0,
		-2.0, -2.0, -2.0,
		-2.0,  2.0, -2.0,
		//
		-2.0, -2.0, -2.0,
		-2.0, -2.0,  2.0,
		-2.0,  2.0,  2.0,
		-2.0,  2.0,  2.0,
		-2.0,  2.0, -2.0,
		-2.0, -2.0, -2.0,
		//
		-2.0, -2.0, -2.0,
		2.0, -2.0, -2.0,
		2.0, -2.0,  2.0,
		2.0, -2.0,  2.0,
		-2.0, -2.0,  2.0,
		-2.0, -2.0, -2.0
	};*/

	const GLfloat lvertices[18] = {
		-10.0, -10.0, -10.0,
		-10.0, -10.0,  10.0,
		-10.0,  10.0,  10.0,
		-10.0,  10.0,  10.0,
		-10.0,  10.0, -10.0,
		-10.0, -10.0, -10.0
	};

	const GLfloat rvertices[18] = {
		-10.0,  10.0, -10.0,
		10.0,  10.0, -10.0,
		10.0, -10.0, -10.0,
		10.0, -10.0, -10.0,
		-10.0, -10.0, -10.0,
		-10.0,  10.0, -10.0
	};

	const GLfloat bvertices[18] = {
		-10.0, -10.0, -10.0,
		10.0, -10.0, -10.0,
		10.0, -10.0,  10.0,
		10.0, -10.0,  10.0,
		-10.0, -10.0,  10.0,
		-10.0, -10.0, -10.0
	};

	/*
	const GLfloat uvs[36] = {
		// back
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		// bottom
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		// left
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
	};
	*/

	const GLfloat luvs[12] = {
		1.0f-0.0f, 1.0f - 1.0f,
		1.0f-1.0f, 1.0f - 1.0f,
		1.0f-1.0f, 1.0f - 0.0f,
		1.0f-1.0f, 1.0f - 0.0f,
		1.0f-0.0f, 1.0f - 0.0f,
		1.0f-0.0f, 1.0f - 1.0f
	};

	const GLfloat ruvs[12] = {
		1.0f - 1.0f, 1.0f - 0.0f,
		1.0f - 0.0f, 1.0f - 0.0f,
		1.0f - 0.0f, 1.0f - 1.0f,
		1.0f - 0.0f, 1.0f - 1.0f,
		1.0f - 1.0f, 1.0f - 1.0f,
		1.0f - 1.0f, 1.0f - 0.0f
	};

	const GLfloat buvs[12] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};
};

#endif