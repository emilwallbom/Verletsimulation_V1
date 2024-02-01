#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;

// Define constants
const size_t MAX_QUADS = 1000;
const size_t MAX_VERTICES = MAX_QUADS * 4;
const size_t MAX_INDICES = MAX_QUADS * 6;

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

    void update_quad(float x, float y, float size);

public:
    Verlet_o(vec2 pos, vec2 acc);

    void updatePosition(float dt);

    inline void accelerate(vec2 force) { acc += force; }

    inline Vertex *get_vertices() { return quad; }

    inline vec2 get_velocity(float dt) { return (curr_pos - prev_pos) / dt; }

    inline void set_velocity(vec2 v) { prev_pos = curr_pos - v; }

    inline float get_radius() { return radius; }

    inline void set_pos(vec2 pos) { curr_pos = pos; }

    inline vec2 get_pos() { return curr_pos; }
};

class Solver
{
private:
    std::vector<Verlet_o> objects;
    std::array<Vertex, MAX_VERTICES> all_vertices;
    int index_count = 0;
    const vec2 gravity = {0.0f, -1000.0f};

    void update_positions(float dt);

    void apply_gravity();

    void apply_window_constraint(float dt);

    void apply_circular_constraint();

    void solve_collisions();

public:
    void add_verlet_object(Verlet_o object);

    void update(float dt, int VBO);

    inline int get_index_count() { return index_count; }
};