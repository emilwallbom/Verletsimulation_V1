#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/window.hpp>

GLFWwindow *window;

#include "solver.hpp"

/*
Window has coordinate system:
(-1000,1000)	(1000,1000)
	+-----------+
	|			|
	|	(0,0)	|
	|			|
	+-----------+
(-1000,-1000)	(1000,-1000)

(Change in vertex shader)
*/

void add_ball_on_press(Solver &solver, bool &once)
{
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && once)
	{
		solver.add_verlet_object(Verlet_o({-800.0f, 800.0f}, {0.0f, 0.0f}));

		// once = false;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		once = true;
	}
}

int main(void)
{
	if (window_init() == -1)
		return -1;

	GLuint shaderProgram = LoadShaders("../src/shaders/VertexShader.vs", "../src/shaders/FragmentShader.fs");
	glUseProgram(shaderProgram);

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
	glEnableVertexAttribArray(0);

	GLuint indices[MAX_INDICES];
	for (size_t i = 0; i < MAX_INDICES; i += 6)
	{
		indices[i + 0] = 0 + i / 6 * 4;
		indices[i + 1] = 1 + i / 6 * 4;
		indices[i + 2] = 2 + i / 6 * 4;
		indices[i + 3] = 2 + i / 6 * 4;
		indices[i + 4] = 3 + i / 6 * 4;
		indices[i + 5] = 0 + i / 6 * 4;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	Solver solver;
	double dt = 0.0;
	double second, t, t_previous = glfwGetTime();
	int frameCount = 0;
	bool once = true;
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	do
	{
		t = glfwGetTime();
		dt = t - t_previous;
		t_previous = t;
		frameCount++;
		// If a second has passed
		if (t - second >= 1.0)
		{
			std::cout << frameCount << std::endl;

			frameCount = 0;
			second = t;
		}

		add_ball_on_press(solver, once);

		solver.update(dt, VBO);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, solver.get_index_count(), GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			 glfwWindowShouldClose(window) == 0);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);
	glfwTerminate();

	return 0;
}
