#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "obj.hpp"

//-----------------------------------------------------------------------------

class entity
{
protected:

    int file;

    float p[3];
    float r[3];

public:

    entity(float x=0, float y=0, float z=0, int f=-1);

    void move(float, float, float);
    void turn(float, float, float);

    void mult_Ri();
    void mult_Ti();

    void mult_S();
    void mult_R();
    void mult_T();
    void mult_M();
    void mult_V();

    virtual void draw(bool=false);

    virtual ~entity();
};

//-----------------------------------------------------------------------------

#endif
