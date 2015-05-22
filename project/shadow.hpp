//
//  shadow.hpp
//  Domescape
//
//  Created by Adam Alsegård on 2015-05-14.
//

#ifndef __Domescape__shadow__
#define __Domescape__shadow__

#include <stdio.h>
#include <string>
#include <vector>
#include <iterator>
#include <glm/glm.hpp>
#include "sgct.h"
#include "../GLFW/glfw3.h"
#include "model.hpp"

class shadow
{
public:

    shadow();
    ~shadow();

    void drawScene(glm::mat4 depthMVP, std::vector<model> objects);

    void clearBuffers();
    void createFBOs(GLint fb_w, GLint fb_h);
    void resizeFBOs();
    void createTexture();
    void setShadowTex(GLint Loc);
    void shadowpass();
    void initPrintMap();
    void printMap();

    GLuint shadowTexture;
    GLuint fbo;


private:

	GLint width;
	GLint height;

	//For visualization of shadowmap
	GLuint quad_vertexbuffer;
	GLint passThroughTex_Loc;


};

#endif /* defined(__Domescape__shadow__) */
