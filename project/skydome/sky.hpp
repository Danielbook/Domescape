#ifndef SKY_HPP
#define SKY_HPP

#include <vector>

#include "entity.hpp"
#include "image.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

class sprite
{
    float x;
    float y;
    float z;
    float s;

public:

    sprite(float a, float b, float c, float d) : x(a), y(b), z(c), s(d) { }

    void draw();
};

class random_sprite : public sprite
{
    float rnd(float a, float z) { return a + (z - a) * rand() / RAND_MAX; }

public:

    random_sprite() : sprite(rnd(0.0f, 180.0f),
                             rnd(0.0f, 360.0f),
                             rnd(0.0f, 360.0f),
                             rnd(1.0f,   2.0f)) { }
};


class sky
{
    image  sun_map;
    image  moon_map;
    image  star_map;
    image  sky_glow;
    image  sky_color;

    sprite sun;
    sprite moon;

    std::vector<random_sprite> stars;

public:

    sky(data *d);

    void draw();
};

//-----------------------------------------------------------------------------

#endif
