#ifndef _CUBE_H_
#define _CUBE_H_

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
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cube
{
public:
	Cube();
	~Cube();

	glm::mat4 toWorld;

	void draw(GLuint, glm::mat4 P, glm::mat4 V);
	void render(GLuint, glm::mat4 P, glm::mat4 V, GLuint FBO);
	void update();
	GLuint loadCubemap();
	unsigned char* loadPPM(const char* filename, int& width, int& height);

	// These variables are needed for the shader program
	GLuint VBO, VAO, uv_ID;
	GLuint uProjection, uModel, uView, texture_ID;

	const GLfloat vertices[108] = {
		-2.0, -2.0,  2.0, // triangle 1 : begin
		2.0, -2.0,  2.0,
		2.0,  2.0,  2.0, // triangle 1 : end
		2.0,  2.0,  2.0, // triangle 2 : begin
		-2.0,  2.0,  2.0,
		-2.0, -2.0,  2.0, // triangle 2 : end
		//
		2.0, -2.0,  2.0,
		2.0, -2.0, -2.0,
		2.0,  2.0, -2.0,
		2.0,  2.0, -2.0,
		2.0,  2.0,  2.0,
		2.0, -2.0,  2.0,
		//
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
		-2.0, -2.0, -2.0,
		//
		-2.0,  2.0,  2.0,
		2.0,  2.0,  2.0,
		2.0,  2.0, -2.0,
		2.0,  2.0, -2.0,
		-2.0,  2.0, -2.0,
		-2.0,  2.0,  2.0
	};

	const GLfloat uvs[72] = {
		// front
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		// top
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
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
		// right
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};
};

// Define the coordinates and indices needed to draw the cube. Note that it is not necessary
// to use a 2-dimensional array, since the layout in memory is the same as a 1-dimensional array.
// This just looks nicer since it's easy to tell what coordinates/indices belong where.
/*
const GLfloat vertices[8][3] = {
	// "Front" vertices
	{-2.0, -2.0,  2.0}, {2.0, -2.0,  2.0}, {2.0,  2.0,  2.0}, {-2.0,  2.0,  2.0},
	// "Back" vertices
	{-2.0, -2.0, -2.0}, {2.0, -2.0, -2.0}, {2.0,  2.0, -2.0}, {-2.0,  2.0, -2.0}
};
*/

// Note that GL_QUADS is deprecated in modern OpenGL (and removed from OSX systems).
// This is why we need to draw each face as 2 triangles instead of 1 quadrilateral
/*
const GLuint indices[6][6] = {
	// Front face
	{0, 1, 2, 2, 3, 0},
	// Top face
	{1, 5, 6, 6, 2, 1},
	// Back face
	{7, 6, 5, 5, 4, 7},
	// Bottom face
	{4, 0, 3, 3, 7, 4},
	// Left face
	{4, 5, 1, 1, 0, 4},
	// Right face
	{3, 2, 6, 6, 7, 3}
};
*/

/*
const GLfloat uvs[48] = {
	// front
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	// top
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	// back
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	// bottom
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	// left
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	// right
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f
};
*/

#endif

