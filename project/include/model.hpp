//
//  model.h
//  Domescape
//
//  Created by Daniel Böök on 2015-04-24.
//  Modified by Adam Alsegård on 2015-05-14
//

#ifndef __Domescape__model__
#define __Domescape__model__

#include <stdio.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "sgct.h"
#include "../../GLFW/glfw3.h"

class model
{
public:
    // Constructor: Create an object, init all to zero
    model();

    // Destructor: Clean up after you
    ~model();

    // Used in destructor
    void clean();

    // Used for reading obj-files
    void readOBJ(const char* filename, std::string texture);

    // Create a sphere (approximated by polygon segments)
    void createSphere(float radius, int segments);

    // Used to render the model
    void render();
    void drawToDepthBuffer();

    void scale(float sx, float sy, float sz);
    void translate(float tx, float ty, float tz);
    void rotate(float ang, float rx, float ry, float rz);

    glm::mat4 transformations;
    std::string mTextureID;

private:
    // Printing errors in console
    void printError(const char *errtype, const char *errmsg);

    GLuint vao;       // Vertex array object, the main handle for geometry
    int nverts; // Number of vertices in the vertex array
    int ntris;  // Number of triangles in the index array (may be zero)
    GLuint vertexbuffer; // Buffer ID to bind to GL_ARRAY_BUFFER
    GLuint indexbuffer;  // Buffer ID to bind to GL_ELEMENT_ARRAY_BUFFER
    GLfloat *vertexarray; // Vertex array on interleaved format: x y z nx ny nz s t
    GLuint *indexarray;   // Element index array



};

#endif /* defined(__Domescape__model__) */
