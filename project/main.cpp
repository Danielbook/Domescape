#include "sgct.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>
#include "objloader.hpp"
#include <glm/gtc/matrix_inverse.hpp>


sgct::Engine * gEngine;

void myInitOGLFun();
//      |
//      V
void myPreSyncFun();//<---------------------------------┐
//      |                                               |
//      V                                               |
void myPostSyncPreDrawFun(); //                         |
//      |                                               |
//      V                                               |
void myDrawFun();//                                     |
//      |                                               |
//      V                                               |
void myEncodeFun();//                                   |
//      |                                               |
//      V                                               |
void myDecodeFun();//                                   |
//      |                                               |
//      V                                               |
void myCleanUpFun();//                                  |
//      |                                               |
//      V                                               |
void keyCallback(int key, int action);//                |
//      |                                               |
//      V                                               |
void mouseButtonCallback(int button, int action);//     |
//      |                                               ^
//      └-----------------------------------------------┘

float rotationSpeed = 0.1f;
float walkingSpeed = 2.5f;
float runningSpeed = 5.0f;
float jumpingHeight = 10.0f;

//regular functions
void loadModel( std::string filename );

enum VBO_INDEXES { VBO_POSITIONS = 0, VBO_UVS, VBO_NORMALS };
GLuint vertexBuffers[3];
GLuint VertexArrayID = GL_FALSE;
GLsizei numberOfVertices = 0;

//shader locations
GLint MVP_Loc = -1;
GLint NM_Loc = -1;

//variables to share across cluster
sgct::SharedDouble curr_time(0.0);
sgct::SharedBool reloadShader(false);

bool arrowButtons[4];
enum directions { FORWARD = 0, BACKWARD, LEFT, RIGHT };

//Used for running
bool runningButton = false;

//Used for jumping
bool jumpingButton = false;

//to check if left mouse button is pressed
bool mouseLeftButton = false;

/* Holds the difference in position between when the left mouse button
 is pressed and when the mouse button is held. */
double mouseDx = 0.0;

/* Stores the positions that will be compared to measure the difference. */
double mouseXPos[] = { 0.0, 0.0 };

glm::vec3 view(0.0f, 0.0f, 1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);
glm::vec3 pos(0.0f, 0.0f, 0.0f);

sgct::SharedObject<glm::mat4> xform;

int main( int argc, char* argv[] )
{
    gEngine = new sgct::Engine( argc, argv );
    
    gEngine->setInitOGLFunction( myInitOGLFun );
    gEngine->setDrawFunction( myDrawFun );
    gEngine->setPreSyncFunction( myPreSyncFun );
    gEngine->setCleanUpFunction( myCleanUpFun );
    gEngine->setPostSyncPreDrawFunction( myPostSyncPreDrawFun );
    gEngine->setKeyboardCallbackFunction( keyCallback );
    gEngine->setMouseButtonCallbackFunction( mouseButtonCallback );

    
    for(int i=0; i<4; i++)
        arrowButtons[i] = false;

    
    if( !gEngine->init( sgct::Engine::OpenGL_3_3_Core_Profile ) )
    {
        delete gEngine;
        return EXIT_FAILURE;
    }
    
    sgct::SharedData::instance()->setEncodeFunction(myEncodeFun);
    sgct::SharedData::instance()->setDecodeFunction(myDecodeFun);
    
    // Main loop
    gEngine->render();
    
    // Clean up
    delete gEngine;
    
    // Exit program
    exit( EXIT_SUCCESS );
}

void myDrawFun()
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    
    double speed = 25.0;
    
    //create scene transform (animation)
    glm::mat4 scene_mat = glm::translate( glm::mat4(1.0f), glm::vec3( 0.0f, 0.0f, -3.0f) );
    scene_mat = glm::rotate( scene_mat, static_cast<float>( curr_time.getVal() * speed ), glm::vec3(0.0f, -1.0f, 0.0f));
    
    glm::mat4 MVP = gEngine->getActiveModelViewProjectionMatrix() * scene_mat;
    glm::mat3 NM = glm::inverseTranspose(glm::mat3( gEngine->getActiveModelViewMatrix() * scene_mat ));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("box"));
    
    sgct::ShaderManager::instance()->bindShaderProgram( "xform" );
    
    glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix3fv(NM_Loc, 1, GL_FALSE, &MVP[0][0]);
    
    // ------ draw model --------------- //
    glBindVertexArray(VertexArrayID);
    glDrawArrays(GL_TRIANGLES, 0, numberOfVertices );
    glBindVertexArray(GL_FALSE); //unbind
    // ----------------------------------//
    
    sgct::ShaderManager::instance()->unBindShaderProgram();
    
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
}

void myPreSyncFun()
{
    if( gEngine->isMaster() )
    {
        curr_time.setVal( sgct::Engine::getTime() );
        
        if( mouseLeftButton )
        {
            double tmpYPos;
            //get the mouse pos from first window
            sgct::Engine::getMousePos( gEngine->getFocusedWindowIndex(), &mouseXPos[0], &tmpYPos );
            mouseDx = mouseXPos[0] - mouseXPos[1];
        }
        
        else
        {
            mouseDx = 0.0;
        }
        
        static float panRot = 0.0f;
        panRot += (static_cast<float>(mouseDx) * rotationSpeed * static_cast<float>(gEngine->getDt()));
        
        glm::mat4 ViewRotateX = glm::rotate(
                                            glm::mat4(1.0f),
                                            panRot,
                                            glm::vec3(0.0f, 1.0f, 0.0f)); //rotation around the y-axis
        
        view = glm::inverse(glm::mat3(ViewRotateX)) * glm::vec3(0.0f, 0.0f, 1.0f);
        
        glm::vec3 right = glm::cross(view, up);
        
        if( arrowButtons[FORWARD] ) {
            if (runningButton) {
                walkingSpeed = runningSpeed;
            }
            else {
                walkingSpeed = 2.5f;
            }
            pos += (walkingSpeed * static_cast<float>(gEngine->getDt()) * view);
            
        }
        if( arrowButtons[BACKWARD] ) {
            if (runningButton) {
                walkingSpeed = runningSpeed;
            }
            else {
                walkingSpeed = 2.5f;
            }
            pos -= (walkingSpeed * static_cast<float>(gEngine->getDt()) * view);
        }
        if( arrowButtons[LEFT] ) {
            if (runningButton) {
                walkingSpeed = runningSpeed;
            }
            else {
                walkingSpeed = 2.5f;
            }
            pos -= (walkingSpeed * static_cast<float>(gEngine->getDt()) * right);
            
        }
        if( arrowButtons[RIGHT] ) {
            if (runningButton) {
                walkingSpeed = runningSpeed;
            }
            else {
                walkingSpeed = 2.5f;
            }
            
            pos += (walkingSpeed * static_cast<float>(gEngine->getDt()) * right);
            
        }
        
        glm::mat4 result;
        //4. transform user back to original position
        result = glm::translate( glm::mat4(1.0f), sgct::Engine::getDefaultUserPtr()->getPos() );
        //3. apply view rotation
        result *= ViewRotateX;
        //2. apply navigation translation
        result *= glm::translate(glm::mat4(1.0f), pos);
        //1. transform user to coordinate system origin
        result *= glm::translate(glm::mat4(1.0f), -sgct::Engine::getDefaultUserPtr()->getPos());
        
        xform.setVal( result );

    }
}

void myPostSyncPreDrawFun()
{
    if( reloadShader.getVal() )
    {
        sgct::ShaderProgram sp = sgct::ShaderManager::instance()->getShaderProgram( "xform" );
        sp.reload();
        
        //reset locations
        sp.bind();
        
        MVP_Loc = sp.getUniformLocation( "MVP" );
        NM_Loc = sp.getUniformLocation( "NM" );
        GLint Tex_Loc = sp.getUniformLocation( "Tex" );
        glUniform1i( Tex_Loc, 0 );
        
        sp.unbind();
        reloadShader.setVal(false);
    }
}

void myInitOGLFun()
{
    sgct::TextureManager::instance()->setWarpingMode(GL_REPEAT, GL_REPEAT);
    sgct::TextureManager::instance()->setAnisotropicFilterSize(4.0f);
    sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);
    sgct::TextureManager::instance()->loadTexure("box", "box.png", true);
    
    loadModel( "box.obj" );
    
    //Set up backface culling
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); //our polygon winding is counter clockwise
    
    sgct::ShaderManager::instance()->addShaderProgram( "xform",
                                                      "simple.vert",
                                                      "simple.frag" );
    
    sgct::ShaderManager::instance()->bindShaderProgram( "xform" );
    
    MVP_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "MVP" );
    NM_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "NM" );
    GLint Tex_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "Tex" );
    glUniform1i( Tex_Loc, 0 );
    
    sgct::ShaderManager::instance()->unBindShaderProgram();
}

void myEncodeFun()
{
    sgct::SharedData::instance()->writeObj( &xform );
    sgct::SharedData::instance()->writeDouble( &curr_time );
    sgct::SharedData::instance()->writeBool( &reloadShader );
}

void myDecodeFun()
{
    sgct::SharedData::instance()->readObj( &xform );
    sgct::SharedData::instance()->readDouble( &curr_time );
    sgct::SharedData::instance()->readBool( &reloadShader );
}

/*!
	De-allocate data from GPU
	Textures are deleted automatically when using texture manager
	Shaders are deleted automatically when using shader manager
 */
void myCleanUpFun()
{
    if( VertexArrayID )
    {
        glDeleteVertexArrays(1, &VertexArrayID);
        VertexArrayID = GL_FALSE;
    }
    
    if( vertexBuffers[0] ) //if first is created, all has been created.
    {
        glDeleteBuffers(3, &vertexBuffers[0]);
        for(unsigned int i=0; i<3; i++)
            vertexBuffers[i] = GL_FALSE;
    }
}

/*
	Loads obj model and uploads to the GPU
 */
void loadModel( std::string filename )
{
    // Read our .obj file
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    
    //if successful
    if( loadOBJ( filename.c_str(), positions, uvs, normals) )
    {
        //store the number of triangles
        numberOfVertices = static_cast<GLsizei>( positions.size() );
        
        //create VAO
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        
        //init VBOs
        for(unsigned int i=0; i<3; i++)
            vertexBuffers[i] = GL_FALSE;
        glGenBuffers(3, &vertexBuffers[0]);
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ VBO_POSITIONS ] );
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
                              0,                  // attribute
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              reinterpret_cast<void*>(0) // array buffer offset
                              );
        
        if( uvs.size() > 0 )
        {
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ VBO_UVS ] );
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
            // 2nd attribute buffer : UVs
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(
                                  1,                                // attribute
                                  2,                                // size
                                  GL_FLOAT,                         // type
                                  GL_FALSE,                         // normalized?
                                  0,                                // stride
                                  reinterpret_cast<void*>(0) // array buffer offset
                                  );
        }
        else
            sgct::MessageHandler::instance()->print("Warning: Model is missing UV data.\n");
        
        if( normals.size() > 0 )
        {
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ VBO_NORMALS ] );
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
            // 3nd attribute buffer : Normals
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(
                                  2,                                // attribute
                                  3,                                // size
                                  GL_FLOAT,                         // type
                                  GL_FALSE,                         // normalized?
                                  0,                                // stride
                                  reinterpret_cast<void*>(0) // array buffer offset
                                  );
        }
        else
            sgct::MessageHandler::instance()->print("Warning: Model is missing normal data.\n");
        
        glBindVertexArray(GL_FALSE); //unbind VAO
        
        //clear vertex data that is uploaded on GPU
        positions.clear();
        uvs.clear();
        normals.clear();
        
        //print some usefull info
        sgct::MessageHandler::instance()->print("Model '%s' loaded successfully (%u vertices, VAO: %u, VBOs: %u %u %u).\n",
                                                filename.c_str(),
                                                numberOfVertices,
                                                VertexArrayID,
                                                vertexBuffers[VBO_POSITIONS],
                                                vertexBuffers[VBO_UVS],
                                                vertexBuffers[VBO_NORMALS] );
    }
    else
        sgct::MessageHandler::instance()->print("Failed to load model '%s'!\n", filename.c_str() );
    
}

void keyCallback(int key, int action)
{
    if( gEngine->isMaster() )
    {
        switch( key )
        {
            case SGCT_KEY_R:
                if(action == SGCT_PRESS)
                    reloadShader.setVal(true);
                break;
            case SGCT_KEY_UP:
            case SGCT_KEY_W:
                arrowButtons[FORWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                printf("W is pressed\n");
                break;
                
            case SGCT_KEY_DOWN:
            case SGCT_KEY_S:
                arrowButtons[BACKWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                printf("S is pressed\n");
                break;
                
            case SGCT_KEY_LEFT:
            case SGCT_KEY_A:
                arrowButtons[LEFT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                printf("A is pressed\n");
                break;
                
            case SGCT_KEY_RIGHT:
            case SGCT_KEY_D:
                arrowButtons[RIGHT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                printf("D is pressed\n");
                break;
                
                //Jumping
            case SGCT_KEY_SPACE:
                jumpingButton = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                printf("Space is pressed\n");
                break;
                
                //Running
            case SGCT_KEY_LEFT_SHIFT:
            case SGCT_KEY_RIGHT_SHIFT:
                runningButton = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                printf("Shift is pressed\n");
                break;

        }
    }
}

void mouseButtonCallback(int button, int action)
{
    if( gEngine->isMaster() )
    {
        switch( button ) {
            case SGCT_MOUSE_BUTTON_LEFT:
                mouseLeftButton = (action == SGCT_PRESS ? true : false);
                double tmpYPos;
                //set refPos
                sgct::Engine::getMousePos(gEngine->getFocusedWindowIndex(), &mouseXPos[1], &tmpYPos);
                break;
        }
    }
}