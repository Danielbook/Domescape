//
//  shadow.cpp
//  Domescape
//
//  Created by Adam Alsegård on 2015-05-14.
//


#include <vector>
#include <iterator>
#include "shadow.hpp"
#include "sgct.h"

// Constructor (används ej)
shadow::shadow()
{

}
//Destructor
shadow::~shadow()
{
    clearBuffers();
    delete mEngine;
}

//Initialize Framebuffers and shadowmap-texture
void shadow::createFBOs(sgct::Engine* engine, GLint fb_w, GLint fb_h)
{

    mEngine = engine;
    fbo = 0;
    shadowTexture = 0;
    width = fb_w;
    height = fb_h;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//create targets
	createTexture();

    // No color output in the bound framebuffer, only depth.
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);


    //Does the GPU support current FBO configuration?
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {    sgct::MessageHandler::instance()->print("FrameBuffer in bad state!\n");   }


    //unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//Create Shadowmap-texture
void shadow::createTexture()
{
	glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &shadowTexture);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); //Sista -> GL_LINEAR?
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	mEngine->checkForOGLErrors();
	//sgct::MessageHandler::instance()->print("%d target textures created.\n");

	glDisable(GL_TEXTURE_2D);
}



void shadow::resizeFBOs()
{
	sgct::MessageHandler::instance()->print("Re-sizing buffers\n");

	GLint W = width;
	GLint H = height;
	//Delete FBO
	clearBuffers();

	//Create resized FBO
	createFBOs(mEngine, W, H);
}


void shadow::clearBuffers()
{

    //delete buffers
    glDeleteFramebuffers(1,	&fbo);

    //delete textures
    glDeleteTextures(1,	&shadowTexture);

    fbo = 0;
    shadowTexture = 0;
    width = 1;
    height = 1;
}


void shadow::setShadowTex(unsigned int index, GLint Loc)
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glUniform1i(Loc, 1);
}


void shadow::shadowpass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT); // Stämmer detta?
    glEnable(GL_BACK);

    //CLear the screen, only depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);

}


/*
    // -----------------------------------------
	// Setup rendering shadow map to screen
	// -----------------------------------------

	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	//Initialize Shader passThroughShadowmap
    sgct::ShaderManager::instance()->addShaderProgram( "passThroughShadowmap", "shaders/passThroughShadow.vert", "shaders/passThroughShadow.frag" );
    sgct::ShaderManager::instance()->bindShaderProgram( "passThroughShadowmap" );

    texID_Loc = sgct::ShaderManager::instance()->getShaderProgram( "passThroughShadowmap").getUniformLocation( "texShadow" );
    //glUniform1i( texID_Loc, 0 );

    sgct::ShaderManager::instance()->unBindShaderProgram();
*/

    // -----------------------------------------
    // Optionally render the shadowmap (for debug only)
    // -----------------------------------------

		// Render only on a corner of the window (or we we won't see the real rendering...)
		//glViewport(0,0,256,256);
		//Bind Shader passThroughShadowmap
  /*      sgct::ShaderManager::instance()->bindShaderProgram( "passThroughShadowmap" );

		glActiveTexture(GL_TEXTURE0);				// Bind our texture in Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(texID_Loc, 0);					// Set our "renderedTexture" sampler to user Texture Unit 0

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle !
		// You have to disable GL_COMPARE_R_TO_TEXTURE above in order to see anything !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		glDisableVertexAttribArray(0);

		sgct::ShaderManager::instance()->unBindShaderProgram();
*/

