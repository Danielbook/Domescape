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
    void createFBOs(sgct::Engine* engine, GLint fb_w, GLint fb_h);
    void resizeFBOs();
    void createTexture();
    void setShadowTex(unsigned int index, GLint Loc);
    void shadowpass();

    GLuint shadowTexture;
    GLuint fbo;


private:

    sgct::Engine* mEngine;

	GLint width;
	GLint height;


};

#endif /* defined(__Domescape__shadow__) */
