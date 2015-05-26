//
//  shadow.cpp
//  Domescape
//
//  Created by Adam Alsegård on 2015-05-14.
//


#include <vector>
#include <iterator>
#include "../include/shadow.hpp"
#include "sgct.h"

// Constructor (används ej)
shadow::shadow()
{

}
//Destructor
shadow::~shadow()
{
    clearBuffers();
}

//Initialize Framebuffers and shadowmap-texture
void shadow::createFBOs(GLint fb_w, GLint fb_h)
{


    fbo = -1;
    shadowTexture = -1;
    width = fb_w;
    height = fb_h;
    quad_vertexbuffer = 0;
	passThroughTex_Loc = -1;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//create targets
	createTexture();

    // No color output in the bound framebuffer, only depth.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexture, 0);
    glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_LINEAR bra tsm med sampler2dShadow, inte Sampler2D -> GL_ NEAREST
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE); //Behövs för Sampler2DShadow men inte debuggande :(

	//mEngine->checkForOGLErrors();
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
	createFBOs(W, H);
}


void shadow::clearBuffers()
{

    //delete buffers
    glDeleteFramebuffers(1,	&fbo);
    glDeleteBuffers(1, &quad_vertexbuffer);

    //delete textures
    glDeleteTextures(1,	&shadowTexture);

    width = 1;
    height = 1;
}



void shadow::setShadowTex( GLint Loc)
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glUniform1i(Loc, 1);
}


void shadow::shadowpass()
{

    //glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); // Stämmer detta?
    //glCullFace(GL_BACK);

}

void shadow::initPrintMap()
{
    // The quad's FBO. Used only for visualizing the shadowmap.
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


	//Initialize Shader depthShadowmap
    sgct::ShaderManager::instance()->addShaderProgram( "passthroughShadowmap", "shaders/passthroughShadow.vert", "shaders/passthroughShadow.frag" );
    sgct::ShaderManager::instance()->bindShaderProgram( "passthroughShadowmap" );

    passThroughTex_Loc = sgct::ShaderManager::instance()->getShaderProgram( "passthroughShadowmap").getUniformLocation( "depthTexture" );
    glUniform1i( passThroughTex_Loc, 0 );

    sgct::ShaderManager::instance()->unBindShaderProgram();

}

//Render Shadowmap for debug
void shadow::printMap()
{

		glViewport(0, 0, width/5, height/5);

		// Use our shader
		sgct::ShaderManager::instance()->bindShaderProgram( "passthroughShadowmap" );

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowTexture);
		// Set our "renderedTexture" sampler to user Texture Unit 0
		glUniform1i(passThroughTex_Loc, 0);

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

}


