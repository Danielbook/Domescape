//
//  model.h
//  Domescape
//
//  Created by Daniel Böök on 2015-04-24.
//
//

#ifndef __Domescape__model__
#define __Domescape__model__

#include <stdio.h>
#include <string>
#include "sgct.h"
#include "GLFW/glfw3.h"

class model
{
private:
    GLuint vao;          // Vertex array object, the main handle for geometry
    int nverts; // Number of vertices in the vertex array
    int ntris;  // Number of triangles in the index array (may be zero)
    GLuint vertexbuffer; // Buffer ID to bind to GL_ARRAY_BUFFER
    GLuint indexbuffer;  // Buffer ID to bind to GL_ELEMENT_ARRAY_BUFFER
    GLfloat *vertexarray; // Vertex array on interleaved format: x y z nx ny nz s t
    GLuint *indexarray;   // Element index array

public:
    // Constructor: Create an object, init all to zero
    model();

    // Destructor: Clean up after you
    ~model();

    // Used in destructor
    void clean();

    // Used for reading obj-files
    void readOBJ(const char* filename);

    // Used to render the model
    void render();


private:
    // Printing errors in console
    void printError(const char *errtype, const char *errmsg);

};

#endif /* defined(__Domescape__model__) */

