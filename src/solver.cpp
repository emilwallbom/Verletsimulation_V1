#include "solver.hpp"

// Verlet object functions
Verlet_o::Verlet_o(vec2 pos, vec2 acc)
{
    this->curr_pos = pos;
    this->prev_pos = pos;
    this->acc = acc;
    update_quad(pos.x, pos.y, radius);
}

void Verlet_o::update_quad(float x, float y, float size)
{
    quad[0] = {{x - size, y - size}};
    quad[1] = {{x + size, y - size}};
    quad[2] = {{x + size, y + size}};
    quad[3] = {{x - size, y + size}};
}

void Verlet_o::updatePosition(float dt)
{
    const vec2 velocity = curr_pos - prev_pos;
    prev_pos = curr_pos;
    curr_pos += velocity + acc * dt * dt;
    update_quad(curr_pos.x, curr_pos.y, radius);
    acc = {};
}

// Solver functions
void Solver::update_positions(float dt)
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

void Solver::apply_gravity()
{
    for (auto &object : objects)
    {
        object.accelerate(gravity);
    };
}

void Solver::apply_window_constraint(float dt)
{
    for (auto &object : objects)
    {
        // Hard coded for now
        const vec2 cons_center = {0.0f, 0.0f};
        const vec2 to_obj = cons_center - object.get_pos();
        const float dist = length(to_obj);
        const vec2 v = object.get_velocity(dt);

        if (object.get_pos()[1] < -1000 + object.get_radius())
            object.set_pos({object.get_pos()[0], -1000 + object.get_radius()});

        if (object.get_pos()[1] > 1000 - object.get_radius())
            object.set_pos({object.get_pos()[0], 1000 - object.get_radius()});

        if (object.get_pos()[0] < -1000 + object.get_radius())
            object.set_pos({-1000 + object.get_radius(), object.get_pos()[1]});

        if (object.get_pos()[0] > 1000 - object.get_radius())
            object.set_pos({1000 - object.get_radius(), object.get_pos()[1]});
    }
}

void Solver::apply_circular_constraint()
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

void Solver::solve_collisions()
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

void Solver::add_verlet_object(Verlet_o object)
{
    object.set_velocity({5.0f, 0.0f});
    objects.push_back(object);

    index_count += 6;
}

void Solver::update(float dt, int VBO)
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
