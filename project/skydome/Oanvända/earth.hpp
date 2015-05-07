#ifndef EARTH_HPP
#define EARTH_HPP

#include "entity.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

class earth : public entity
{
public:

    earth(data *d);

    void draw(bool);
};

//-----------------------------------------------------------------------------

#endif
