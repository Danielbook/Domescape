#ifndef BUGS_HPP
#define BUGS_HPP

#include <vector>

#include "entity.hpp"
#include "opengl.hpp"
#include "image.hpp"
#include "conf.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

class bug
{
    float x;
    float y;
    float z;
    float r;

    float rnd(float a, float z) { return a + (z - a) * rand() / RAND_MAX; }

public:

    bug() : x(rnd(0.0f, 360.0f)),
            y(rnd(0.0f, 360.0f)),
            z(rnd(0.0f, 360.0f)),
            r(rnd(0.5f,   2.0f)) { }

    void step(float);
    void draw();
};

//-----------------------------------------------------------------------------

class swarm : public entity
{
    static const int max_count = 20;

    image bug_map;
    float count;

    std::vector<bug> bugs;

public:

    swarm(conf *c, data *d, float, float, float);

    void size(float);
    void step(float);
    void draw(bool);
};

//-----------------------------------------------------------------------------

#endif
