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

// Define constants
const size_t MAX_QUADS = 1000;
const size_t MAX_VERTICES = MAX_QUADS * 4;
const size_t MAX_INDICES = MAX_QUADS * 6;

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
struct Vertex
{
	float pos[2];
};

class Verlet_o
{
private:
	vec2 curr_pos;
	vec2 prev_pos;
	vec2 acc;
	Vertex quad[4];
	size_t radius = 10.0f;

	void update_quad(float x, float y, float size)
	{
		quad[0] = {{x - size, y - size}};
		quad[1] = {{x + size, y - size}};
		quad[2] = {{x + size, y + size}};
		quad[3] = {{x - size, y + size}};
	}

public:
	Verlet_o(vec2 pos, vec2 acc)
	{
		this->curr_pos = pos;
		this->prev_pos = pos;
		this->acc = acc;
		update_quad(pos.x, pos.y, radius);
	}

	void updatePosition(float dt)
	{
		const vec2 velocity = curr_pos - prev_pos;
		prev_pos = curr_pos;
		curr_pos += velocity + acc * dt * dt;
		update_quad(curr_pos.x, curr_pos.y, radius);
		acc = {};
	}

	void accelerate(vec2 force)
	{
		acc += force;
	}

	Vertex *get_vertices()
	{
		return quad;
	}

	vec2 get_velocity(float dt)
	{
		return (curr_pos - prev_pos) / dt;
	}

	vec2 set_velocity(vec2 v)
	{
		prev_pos = curr_pos - v;
	}

	float get_radius()
	{
		return radius;
	}

	void set_pos(vec2 pos)
	{
		curr_pos = pos;
	}

	vec2 get_pos()
	{
		return curr_pos;
	}
};

class Solver
{
private:
	std::vector<Verlet_o> objects;
	std::array<Vertex, MAX_VERTICES> all_vertices;
	int index_count = 0;
	const vec2 gravity = {0.0f, -1000.0f};

	void update_positions(float dt)
	{
		int i = 0;
		for (auto &object : objects)
		{
			object.updatePosition(dt);
			all_vertices[i] = object.get_vertices()[0];
			all_vertices[i + 1] = object.get_vertices()[1];
			all_vertices[i + 2] = object.get_vertices()[2];
			all_vertices[i + 3] = object.get_vertices()[3];
			i += 4;
		}
	}

	void apply_gravity()
	{
		for (auto &object : objects)
		{
			object.accelerate(gravity);
		};
	}

	void apply_window_constraint(float dt)
	{
		for (auto &object : objects)
		{
			// Hard coded for now
			const vec2 cons_center = {0.0f, 0.0f};
			const vec2 to_obj = cons_center - object.get_pos();
			const float dist = length(to_obj);
			if (object.get_pos()[1] < -1000 + object.get_radius())
			{
				const vec2 v = object.get_velocity(dt);

				object.set_pos({object.get_pos()[0], -1000 + object.get_radius()});
			}
			if (object.get_pos()[1] > 1000 - object.get_radius())
			{
				const vec2 v = object.get_velocity(dt);

				object.set_pos({object.get_pos()[0], 1000 - object.get_radius()});
			}
			if (object.get_pos()[0] < -1000 + object.get_radius())
			{
				const vec2 v = object.get_velocity(dt);

				object.set_pos({-1000 + object.get_radius(), object.get_pos()[1]});
			}
			if (object.get_pos()[0] > 1000 - object.get_radius())
			{
				const vec2 v = object.get_velocity(dt);

				object.set_pos({1000 - object.get_radius(), object.get_pos()[1]});
			}
		}
	}
	void apply_circular_constraint()
	{
		const vec2 cons_center = {0.0f, 0.0f};
		const float cons_radius = 1000.0f;

		for (auto &object : objects)
		{
			const vec2 to_obj = cons_center - object.get_pos();
			const float dist = length(to_obj);
			if (dist > cons_radius - object.get_radius())
			{
				const vec2 n = normalize(to_obj);
				object.set_pos(cons_center - n * (cons_radius - object.get_radius()));
			}
		}
	}

	void solve_collisions()
	{
		for (auto &object : objects)
		{
			for (auto &other : objects)
			{
				if (&object == &other)
					continue;
				const vec2 to_obj = object.get_pos() - other.get_pos();
				const float dist = length(to_obj);
				if (dist < object.get_radius() + other.get_radius())
				{
					const vec2 n = normalize(to_obj);
					const float delta = object.get_radius() + other.get_radius() - dist;
					object.set_pos(object.get_pos() + n * delta / 2.0f);
					other.set_pos(other.get_pos() - n * delta / 2.0f);
				}
			}
		}
	}

public:
	void add_verlet_object(Verlet_o object)
	{
		object.set_velocity({5.0f, 0.0f});
		objects.push_back(object);

		index_count += 6;
	}

	void update(float dt, int VBO)
	{
		const int sub_steps = 4;
		const float sub_dt = dt / sub_steps;
		for (size_t i(sub_steps); i--;)
		{
			apply_gravity();
			apply_window_constraint(sub_dt);
			solve_collisions();
			update_positions(sub_dt);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * all_vertices.size(), all_vertices.data());
		}
	}

	int get_index_count()
	{
		return index_count;
	}
};

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
