#include "sgct.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <string>
#include <algorithm>
#include <vector>
#include <iterator>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

//For the time function
#include <time.h>

#include <SpiceUsr.h>
#include <SpiceZfc.h>

#include "include/model.hpp"
#include "include/shadow.hpp" //Har även ändrat include i shadow.cpp
#include "include/shader.hpp"


sgct::Engine * gEngine;

void myInitOGLFun();
//      |
//      V
void myPreSyncFun();//<---------------------------------┐
//      |                                               |
//      V                                               |
//////SYNC//////////                                    |
//      |                                               |
//      V                                               |
void myPostSyncPreDrawFun(); //                         |
//      |                                               |
//      V<-------------|                                |
//////CLEAR BUFFERS/   |                                |
//      |              |                                |
//      V              |                                |
void myDrawFun();//    |                                |
//      |--------------|                                |
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


/*------------------MOVEMENT------------------*/
float rotationSpeed = 0.1f;
float walkingSpeed = 2.5f;
float runningSpeed = 5.0f;

bool dirButtons[6];
enum directions { FORWARD = 0, BACKWARD = 1, LEFT = 2, RIGHT = 3, UP = 4, DOWN = 5 };

bool runningButton = false;
bool mouseLeftButton = false;

double mouseDx = 0.0;
double mouseDy = 0.0;

double mouseXPos[] = { 0.0, 0.0 };
double mouseYPos[] = { 0.0, 0.0 };

glm::vec3 bView(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);
glm::vec3 pos(0.0f, 0.0f, 0.0f);
/*--------------------------------------------*/

/*------------------REGULAR FUNCTIONS------------------*/
void calcSunPosition(); // Calculates the suns position
void calcSkyColor(float fSunAnglePhi, float &fAmb, glm::vec4 &sColor);
void resetToCurrentTime(); // Used to calculate the time of the current computer
void addSecondToTime();
/*-----------------------------------------------------*/

/*------------------SHADOWMAP------------------*/
//Shader Locations
GLint depthMVP_Loc = -1;
glm::mat4 depthMVP;

//For several windows
std::vector<class shadow> buffers;
//Singular shader
shadow myShadow;
//SGCT - solution
sgct_core::OffScreenBuffer *myBuffer;
/*---------------------------------------------*/

/*------------------SHADERS------------------*/
//Shader Scene
GLint MVP_Loc = -1;
GLint NM_Loc = -1;
GLint sColor_Loc = -1;
GLint lDir_Loc = -1;
GLint Amb_Loc = -1;
GLint Tex_Loc = -1;

GLint depthBiasMVP_Loc = -1;
GLint shadowmap_Loc = -1;

//Shader Sky
GLint MVP_Loc_S = -1;
GLint NM_Loc_S = -1;
GLint lDir_Loc_S = -1;
GLint Tex_Loc_S = -1;
GLint Glow_Loc_S = -1;
GLint SunColor_Loc_S = -1;
/*------------------------------------------*/

/*------------------SHARED VARIABLES ACROSS THE CLUSTER------------------*/
sgct::SharedDouble curr_time(0.0);
sgct::SharedBool reloadShader(false);
sgct::SharedObject<glm::mat4> xform;
/*-----------------------------------------------------------------------*/

/*------------------GUI------------------*/
void externalControlMessageCallback(const char * receivedChars, int size);
void externalControlStatusCallback(bool connected);

sgct::SharedBool timeIsTicking(true);
sgct::SharedInt timeSpeed = 1;
sgct::SharedString date;
sgct::SharedBool oneSecondPassed(false);
/*---------------------------------------*/

/*---------------OTHER VARIABLES--------------*/
//SUN POSITION
float fSunAnglePhi;
float fSunAngleTheta;
float fAmb = 0.2f; //Initialize to low for debugging purposes
glm::vec4 sColor = glm::vec4(0.4f, 0.4f, 0.4f, 0.4f); //Initialize to low for debugging purposes
glm::vec3 lDir;
glm::vec3 vSunPos;

//TIME
enum timeVariables{YEAR = 0, MONTH = 1, DAY = 2, HOUR = 3, MINUTE = 4, SECOND = 5};
int currentTime[6];
int timeCount = 0;

//OBJECTS
model landscape;
model box;
model sun;
model skyDome;
sgct_utils::SGCTDome* newDome;

// Array with all models
const int numberOfObjects = 2;
model listObj[numberOfObjects];

glm::mat4 nyDepthMVP;
glm::mat4 nyMVP;
/*------------------------------------------------*/

int main( int argc, char* argv[] ){
    gEngine = new sgct::Engine( argc, argv );

    gEngine->setInitOGLFunction( myInitOGLFun );
    gEngine->setPreSyncFunction( myPreSyncFun );
    gEngine->setPostSyncPreDrawFunction( myPostSyncPreDrawFun );
    gEngine->setDrawFunction( myDrawFun );
    gEngine->setCleanUpFunction( myCleanUpFun );
    gEngine->setKeyboardCallbackFunction( keyCallback );
    gEngine->setMouseButtonCallbackFunction( mouseButtonCallback );
    gEngine->setExternalControlCallback( externalControlMessageCallback );
    gEngine->setExternalControlStatusCallback( externalControlStatusCallback );

    /*------------------GUI------------------*/
    sgct::SharedData::instance()->setEncodeFunction(myEncodeFun);
    sgct::SharedData::instance()->setDecodeFunction(myDecodeFun);
    /*-----------------------------------------*/

    /*------------------SPICE-------------------*/
    /*      Kernels needed for calculations     */
    furnsh_c( "kernels/naif0011.tls" ); //Is a generic kernel that you can use to get the positions of Earth and the Sun for various times
    furnsh_c( "kernels/de430.bsp" );    //Is a leapsecond kernel so that you get the accurate times
    furnsh_c( "kernels/pck00010.tpc" ); //Might also be needed
    /*-----------------------------------------*/

    for(int i=0; i<6; i++)
        dirButtons[i] = false;

    //SHADOWMAP
    //sgct::SGCTSettings::instance()->setUseDepthTexture(true);
    //sgct::SGCTSettings::instance()->setUseFBO(true);
    //myBuffer = new sgct_core::OffScreenBuffer;

#if __APPLE__
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) ){
        delete gEngine;
        return EXIT_FAILURE;
    }

#elif (_MSC_VER >= 1500)
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) ){
        delete gEngine;
        return EXIT_FAILURE;
    }

#elif __WIN32__
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) ){
        delete gEngine;
        return EXIT_FAILURE;
    }

#elif __linux__
    if( !gEngine->init( ) ){
        delete gEngine;
        return EXIT_FAILURE;
    }
#endif

    resetToCurrentTime();

    // Main loop
    gEngine->render();

    // Clean up
    delete gEngine;

    // Exit program
    exit( EXIT_SUCCESS );

    return( 0 );
}

void myInitOGLFun(){
    sgct::TextureManager::instance()->setWarpingMode(GL_REPEAT, GL_REPEAT);
    sgct::TextureManager::instance()->setAnisotropicFilterSize(8.0f);
    sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);

    gEngine->setNearAndFarClippingPlanes(0.1f, 2000.0f);

    /*----------------OBJECTS AND TEXTURES--------------*/
    // OBJECTS TO SKY
    sgct::TextureManager::instance()->loadTexure("sun", "texture/sun.jpg", true);
    sun.createSphere(10.0f, 80);

    //skyDome.createSphere(5.0f, 100);
    //int x, y = 0;
    //gEngine->getActiveViewportSize(x, y);
    //newDome = new sgct_utils::SGCTDome(500, x/y, 100, 20, 0.2f);

    // OBJECTS TO SCENE
    //Transformations from origo. ORDER MATTERS!
    landscape.readOBJ("mesh/landscape2.obj", "texture/landscape2.png");
    landscape.translate(0.0f, -20.0f, 0.0f);
    landscape.scale(1.0f, 1.0f, 1.0f);
    listObj[0] = landscape; // sparar i array

    box.readOBJ("mesh/box.obj", "texture/box.png");
    box.translate(0.0f, 0.0f, -5.0f);
    box.scale(2.0f, 2.0f, 2.0f);
    listObj[1] = box; // sparar i array

    /*----------------------------------------------------------*/

    /*------------------------SHADOWMAP-------------------------*/

	sgct_core::SGCTNode * thisNode = sgct_core::ClusterManager::instance()->getThisNodePtr();
	for(unsigned int i=0; i < thisNode->getNumberOfWindows(); i++)
	{
		class shadow tmpBuffer;
		buffers.push_back( tmpBuffer );
	}
	sgct::MessageHandler::instance()->print("Number of buffers: %d\n", buffers.size());

	for(unsigned int i=0; i < buffers.size(); i++)
	{
        GLint fb_width, fb_height = 0;
        sgct::SGCTWindow * winPtr = gEngine->getWindowPtr(i);
		winPtr->getDrawFBODimensions(fb_width, fb_height);
        buffers[i].createFBOs( fb_width, fb_height);
        gEngine->checkForOGLErrors();

        //buffers[i].initPrintMap();

        //myBuffer->createFBO(fb_width, fb_height);
        //myBuffer->attachDepthTexture(buffers[i].shadowTexture);
        //winPtr->getFrameBufferTexture(i); //Använda denna istället?
    }

	//Initialize Shader depthShadowmap
    sgct::ShaderManager::instance()->addShaderProgram( "depthShadowmap", "shaders/depthShadow.vert", "shaders/depthShadow.frag" );
    sgct::ShaderManager::instance()->bindShaderProgram( "depthShadowmap" );

    depthMVP_Loc = sgct::ShaderManager::instance()->getShaderProgram( "depthShadowmap").getUniformLocation( "depthMVP" );

    sgct::ShaderManager::instance()->unBindShaderProgram();

    /*-----------------------------------------------------------*/

    /*---------------------SHADERS-----------------------*/
    //Initialize Shader scene
    sgct::ShaderManager::instance()->addShaderProgram( "scene", "shaders/scene.vert", "shaders/scene.frag" );
    sgct::ShaderManager::instance()->bindShaderProgram( "scene" );

    MVP_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "MVP" );
    NM_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "NM" );
    sColor_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "sunColor" );
    lDir_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "lightDir" );
    Amb_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "fAmbInt" );
    Tex_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "Tex" );
    glUniform1i( Tex_Loc, 0 );
    depthBiasMVP_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "depthBiasMVP" );
    shadowmap_Loc = sgct::ShaderManager::instance()->getShaderProgram( "scene").getUniformLocation( "shadowMap" );
    glUniform1i( shadowmap_Loc, 1 );

    sgct::ShaderManager::instance()->unBindShaderProgram();


    //Initialize Shader sky (sky)
    sgct::ShaderManager::instance()->addShaderProgram( "sky", "shaders/sky.vert", "shaders/sky.frag" );
    sgct::ShaderManager::instance()->bindShaderProgram( "sky" );

    MVP_Loc_S = sgct::ShaderManager::instance()->getShaderProgram( "sky").getUniformLocation( "MVP" );
    NM_Loc_S = sgct::ShaderManager::instance()->getShaderProgram( "sky").getUniformLocation( "NM" );
    lDir_Loc_S = sgct::ShaderManager::instance()->getShaderProgram( "sky").getUniformLocation( "lightDir" );
    Tex_Loc_S = sgct::ShaderManager::instance()->getShaderProgram( "sky").getUniformLocation( "Tex" );
    Glow_Loc_S = sgct::ShaderManager::instance()->getShaderProgram( "sky").getUniformLocation( "glow" );
    SunColor_Loc_S = sgct::ShaderManager::instance()->getShaderProgram( "sky").getUniformLocation( "colorSky" );
    glUniform1i( Tex_Loc_S, 0 );
    glUniform1i( SunColor_Loc_S, 1 );
    glUniform1i( Glow_Loc_S, 2 );

    sgct::ShaderManager::instance()->unBindShaderProgram();

    /*---------------------------------------------------------*/
}


void myPreSyncFun(){
    if( gEngine->isMaster() ){
        curr_time.setVal( sgct::Engine::getTime() );

        if( mouseLeftButton ){
            //get the mouse pos from first window
            sgct::Engine::getMousePos( gEngine->getFocusedWindowIndex(), &mouseXPos[0], &mouseYPos[0] );
            mouseDx = mouseXPos[0] - mouseXPos[1];
            mouseDy = mouseYPos[0] - mouseYPos[1];
        }
        else{
            mouseDy = 0.0;
            mouseDx = 0.0;
        }

        //MOUSE AND KEYBOARD INPUT
        static float panRot = 0.0f;
        panRot += (static_cast<float>(mouseDx) * rotationSpeed * static_cast<float>(gEngine->getDt()));

        static float tiltRot = 0.0f;
        tiltRot += (static_cast<float>(mouseDy) * rotationSpeed * static_cast<float>(gEngine->getDt()));

        glm::mat4 ViewRotateX = glm::rotate(
                                            glm::mat4(1.0f),
                                            panRot,
                                            glm::vec3(0.0f, 1.0f, 0.0f)); //rotation around the y-axis


        bView = glm::inverse(glm::mat3(ViewRotateX)) * glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 right = glm::cross(bView, up);

        glm::mat4 ViewRotateY = glm::rotate(
                                            glm::mat4(1.0f),
											tiltRot,
											-right); //rotation around the movavble x-axis

        if( dirButtons[FORWARD] ){
            runningButton ? walkingSpeed = runningSpeed: walkingSpeed = 2.5f;
            pos += (walkingSpeed * static_cast<float>(gEngine->getDt()) * bView);
        }
        if( dirButtons[BACKWARD] ){
            runningButton ? walkingSpeed = runningSpeed: walkingSpeed = 2.5f;
            pos -= (walkingSpeed * static_cast<float>(gEngine->getDt()) * bView);
        }
        if( dirButtons[LEFT] ){
            runningButton ? walkingSpeed = runningSpeed: walkingSpeed = 2.5f;
            pos -= (walkingSpeed * static_cast<float>(gEngine->getDt()) * right);
        }
        if( dirButtons[RIGHT] ){
            runningButton ? walkingSpeed = runningSpeed: walkingSpeed = 2.5f;
            pos += (walkingSpeed * static_cast<float>(gEngine->getDt()) * right);
        }
        if( dirButtons[UP] ){
            runningButton ? walkingSpeed = runningSpeed: walkingSpeed = 2.5f;
            pos -= (walkingSpeed * static_cast<float>(gEngine->getDt()) * up);
        }
        if( dirButtons[DOWN] ){
            runningButton ? walkingSpeed = runningSpeed: walkingSpeed = 2.5f;
            pos += (walkingSpeed * static_cast<float>(gEngine->getDt()) * up);
        }

        glm::mat4 result;
        //4. transform user back to original position
        result = glm::translate( glm::mat4(1.0f), sgct::Engine::getDefaultUserPtr()->getPos() );

        //3. apply view rotation
        result *= ViewRotateX;
        result *= ViewRotateY;

        //2. apply navigation translation
        result *= glm::translate(glm::mat4(1.0f), pos);

        //1. transform user to coordinate system origin
        result *= glm::translate(glm::mat4(1.0f), -sgct::Engine::getDefaultUserPtr()->getPos());

        //0. Translate to eye height of a person
        result *= glm::translate( glm::mat4(1.0f), glm::vec3( 0.0f, -1.6f, 0.0f ) );

        xform.setVal( result );
    }
}

void myPostSyncPreDrawFun(){
    if( timeIsTicking.getVal() && oneSecondPassed.getVal() ){
        std::cout << "Time is ticking" << std::endl;
    }

    else if( !timeIsTicking.getVal() && oneSecondPassed.getVal() ){
        std::cout << "Time is paused" << std::endl;
    }

    ////fuLhaxX

    oneSecondPassed.setVal(false);

    if( timeIsTicking.getVal() )
        timeCount++;

    if( timeCount >= 60 ){
        oneSecondPassed.setVal(true);
        timeCount = 0;
    }

    if( oneSecondPassed.getVal() ){

        std::cout << currentTime[YEAR] << " " << currentTime[MONTH] << " " << currentTime[DAY] << " " << currentTime[HOUR] << ":" << currentTime[MINUTE] << ":" << currentTime[SECOND] << std::endl;

        if( timeIsTicking.getVal() ){
            addSecondToTime();
        }
    }
    ///////////


    if( reloadShader.getVal() )
    {
        //Call shader-reload senare
        sgct::ShaderProgram sp = sgct::ShaderManager::instance()->getShaderProgram( "scene" );
        sp.reload();

        //reset locations
        sp.bind();

        MVP_Loc = sp.getUniformLocation( "MVP" );
        NM_Loc = sp.getUniformLocation( "NM" );
        depthBiasMVP_Loc = sp.getUniformLocation( "depthBiasMVP" );
        sColor_Loc = sp.getUniformLocation("sunColor");
        lDir_Loc = sp.getUniformLocation("lightDir");
        Amb_Loc = sp.getUniformLocation("fAmbInt");
        Tex_Loc = sp.getUniformLocation( "Tex" );
        glUniform1i( Tex_Loc, 0 );
        shadowmap_Loc = sp.getUniformLocation( "shadowMap" );
        glUniform1i( shadowmap_Loc, 1);

        sp.unbind();

        sgct::ShaderProgram skySp = sgct::ShaderManager::instance()->getShaderProgram( "sky" );
        skySp.reload();

        //reset locations
        skySp.bind();

        MVP_Loc_S = skySp.getUniformLocation( "MVP" );
        NM_Loc_S = skySp.getUniformLocation( "NM" );
        Tex_Loc_S = skySp.getUniformLocation( "Tex" );
        lDir_Loc_S = skySp.getUniformLocation("lightDir");
        Glow_Loc_S = skySp.getUniformLocation( "glow" );
        SunColor_Loc_S = skySp.getUniformLocation( "colorSky" );
        glUniform1i( Glow_Loc_S, 2 );
        glUniform1i( SunColor_Loc_S, 1 );
        glUniform1i( Tex_Loc_S, 0 );

        skySp.unbind();

        reloadShader.setVal(false);
    }



    //Fisheye cubemaps are constant size
	sgct_core::SGCTNode * thisNode = sgct_core::ClusterManager::instance()->getThisNodePtr();
	for(unsigned int i=0; i < thisNode->getNumberOfWindows(); i++)
	{
		if( gEngine->getWindowPtr(i)->isWindowResized() && !gEngine->getWindowPtr(i)->isUsingFisheyeRendering() )
		{
			buffers[i].resizeFBOs();
            //myShadow.resizeFBOs();

//            GLint fb_width, fb_height = 0;
//            sgct::SGCTWindow * winPtr = gEngine->getWindowPtr(i);
//            winPtr->getDrawFBODimensions(fb_width, fb_height);
//            myBuffer->resizeFBO(fb_width, fb_height);
			break;
		}
    }

    //Kallas endast 1gång/frame till skillnad från draw...
    /*------------------SUNPOSITION-----------------------*/

    // Set light properties
    float fSunDis = 800;

    calcSunPosition();

    if( oneSecondPassed.getVal() ){
        std::cout<<"THETA: "<< fSunAngleTheta << std::endl;
        std::cout<<"PHI: " << fSunAnglePhi << std::endl;
    }

    vSunPos = glm::vec3(fSunDis*sin(fSunAngleTheta)*cos(fSunAnglePhi),fSunDis*sin(fSunAngleTheta)*sin(fSunAnglePhi),fSunDis*cos(fSunAngleTheta));

    calcSkyColor(fSunAnglePhi, fAmb, sColor);

    lDir = glm::normalize(vSunPos);

    /*---------------------------------------------*/

    /*------------------SHADOW MAP------------------*/

    //get a pointer to the current window
	sgct::SGCTWindow * winPtr = gEngine->getActiveWindowPtr();
	unsigned int index = winPtr->getId();
	winPtr->getFBOPtr()->unBind();

    // Compute the MVP matrix from the light's point of view
    //glm::mat4 depthProjectionMatrix = glm::ortho<float>( -100, 100, -100, 100, 0.1, 150);
    glm::mat4 depthProjectionMatrix = gEngine->getActiveProjectionMatrix();
    glm::mat4 depthViewMatrix = glm::lookAt(vSunPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 depthModelMatrix = glm::mat4(1.0);
    depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
    glDepthFunc(GL_ALWAYS);

    //Bind current framebuffer
    buffers[index].shadowpass();
    //myBuffer->bind();

    //CLear the screen, only depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);

    sgct::ShaderManager::instance()->bindShaderProgram( "depthShadowmap" );


    // Loopar igenom alla objekt i arrayen
    for( int i = 0; i < numberOfObjects; ++i)
    {
        nyDepthMVP = depthMVP * listObj[i].transformations;
        glUniformMatrix4fv(depthMVP_Loc, 1, GL_FALSE, glm::value_ptr(nyDepthMVP));
        //listObj[i].drawToDepthBuffer();
        listObj[i].render();
    }

    sgct::ShaderManager::instance()->unBindShaderProgram();


    //Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //myBuffer->unBind();
    winPtr->getFBOPtr()->bind();

    /*------------------------------------------------*/

}

void myDrawFun(){

    //create scene transform (animation)
    glm::mat4 scene_mat = xform.getVal();

    glm::mat4 MV = gEngine->getActiveModelViewMatrix() * scene_mat;
    glm::mat4 MVP = gEngine->getActiveModelViewProjectionMatrix() * scene_mat;
    glm::mat3 NM = glm::inverseTranspose(glm::mat3( MV ));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /*------------------SCENE SHADER------------------*/
    sgct::SGCTWindow * winPtr = gEngine->getActiveWindowPtr();
	unsigned int index = winPtr->getId();
    //for(unsigned int win=0; win < buffers.size(); win++)
	//{

    //Bind Shader scene
    sgct::ShaderManager::instance()->bindShaderProgram( "scene" );

    glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,    0.0, 0.5, 0.0, 0.0,    0.0, 0.0, 0.5, 0.0,    0.5, 0.5, 0.5, 1.0);
    glm::mat4 depthBiasMVP = biasMatrix*depthMVP;

    glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix3fv(NM_Loc, 1, GL_FALSE, &NM[0][0]);
    glUniform4fv(sColor_Loc, 1, &sColor[0]);
    glUniform3fv(lDir_Loc, 1, &lDir[0]);
    glUniform1fv(Amb_Loc, 1, &fAmb);
    glUniformMatrix4fv(depthBiasMVP_Loc, 1, GL_FALSE, &depthBiasMVP[0][0]);


    // Loopar igenom alla objekt i arrayen
    for( int i = 0; i < numberOfObjects; ++i)
    {
        nyMVP = MVP * listObj[i].transformations;
        glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, glm::value_ptr(nyMVP));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId(listObj[i].mTextureID));
        glUniform1i(Tex_Loc, 0);

        //buffers[index].setShadowTex(shadowmap_Loc);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, buffers[index].shadowTexture);
        glUniform1i(shadowmap_Loc, 1);

        listObj[i].render();
    }

    sgct::ShaderManager::instance()->unBindShaderProgram();

    //Render shadowMap-texturen
    //buffers[index].printMap();
    //}
    /*----------------------------------------------*/

    /*------------------SKY SHADER------------------*/

    //get viewport data and set the viewport
	const int * coords;
	coords = gEngine->getActiveViewportPixelCoords();
	glViewport( coords[0], coords[1], coords[2], coords[3] );

    //Bind Shader sky
    sgct::ShaderManager::instance()->bindShaderProgram( "sky" );

    glUniformMatrix4fv(MVP_Loc_S, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix3fv(NM_Loc_S, 1, GL_FALSE, &NM[0][0]);
    glUniform3fv(lDir_Loc_S, 1, &lDir[0]);

    //SUN
    nyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!
        nyMVP = glm::translate(nyMVP, vSunPos);

        //Send the transformations, texture and render
        glUniformMatrix4fv(MVP_Loc_S, 1, GL_FALSE, glm::value_ptr(nyMVP));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("sun"));
        glUniform1i(Tex_Loc, 0);
        sun.render();


    //SKYDOME
    nyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!

        glUniformMatrix4fv(MVP_Loc_S, 1, GL_FALSE, glm::value_ptr(nyMVP));
        //newDome->draw();


    sgct::ShaderManager::instance()->unBindShaderProgram();

    /*----------------------------------------------*/

    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
}

void myEncodeFun(){
    sgct::SharedData::instance()->writeObj( &xform );
    sgct::SharedData::instance()->writeDouble( &curr_time );
    sgct::SharedData::instance()->writeBool( &reloadShader );

    //GUI
    sgct::SharedData::instance()->writeBool( &timeIsTicking );
    sgct::SharedData::instance()->writeString( &date );
    sgct::SharedData::instance()->writeInt( &timeSpeed );
    sgct::SharedData::instance()->writeBool( &oneSecondPassed );
}

void myDecodeFun(){
    sgct::SharedData::instance()->readObj( &xform );
    sgct::SharedData::instance()->readDouble( &curr_time );
    sgct::SharedData::instance()->readBool( &reloadShader );

    //GUI
    sgct::SharedData::instance()->readBool( &timeIsTicking );
    sgct::SharedData::instance()->readString( &date );
    sgct::SharedData::instance()->readInt( &timeSpeed );
    sgct::SharedData::instance()->readBool( &oneSecondPassed );
}

/*!
	De-allocate data from GPU
	Textures are deleted automatically when using texture manager
	Shaders are deleted automatically when using shader manager
 */
void myCleanUpFun(){
    for(unsigned int i=0; i < buffers.size(); i++)
	{
        buffers[i].clearBuffers();
    }
    buffers.clear();
    //myBuffer->destroy();
}

void keyCallback(int key, int action){
    if( gEngine->isMaster() ){
        switch( key ){
            case SGCT_KEY_R: if(action == SGCT_PRESS) reloadShader.setVal(true); break;
            case SGCT_KEY_W: case SGCT_KEY_UP: dirButtons[FORWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false); break;
            case SGCT_KEY_S: case SGCT_KEY_DOWN:dirButtons[BACKWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false); break;
            case SGCT_KEY_A: case SGCT_KEY_LEFT: dirButtons[LEFT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false); break;
            case SGCT_KEY_D: case SGCT_KEY_RIGHT:dirButtons[RIGHT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false); break;
 /*Running*/case SGCT_KEY_LEFT_SHIFT: case SGCT_KEY_RIGHT_SHIFT: runningButton = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false); break;
        	case SGCT_KEY_Q: dirButtons[UP] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false); break;
        	case SGCT_KEY_E: dirButtons[DOWN] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false); break;
        }
    }
}

void mouseButtonCallback(int button, int action){
    if( gEngine->isMaster() ){
        switch( button ) {
            case SGCT_MOUSE_BUTTON_LEFT:
                mouseLeftButton = (action == SGCT_PRESS ? true : false);
                //set refPos
                sgct::Engine::getMousePos(gEngine->getFocusedWindowIndex(), &mouseXPos[1], &mouseYPos[1]);
                break;
        }
    }
}

void externalControlMessageCallback(const char * receivedChars, int size){
    if( gEngine->isMaster() ){
        //PAUSE TIME
        if( strncmp(receivedChars, "pause", 5) == 0 ){
            if( strncmp(receivedChars, "pause=0", 7) == 0 ){
                timeIsTicking.setVal( true );
                std::cout << "CONTINUE TIME" << std::endl;
            }
            else if( strncmp(receivedChars, "pause=1", 7) == 0 ){
                timeIsTicking.setVal( false );
                //std::cout << "PAUSE TIME" << std::endl;
            }
        }

        //RESET TO CURRENT TIME
        if( size == 7 && strncmp( receivedChars, "reset", 4 ) == 0 ){
            if( strncmp(receivedChars, "reset=1", 7) == 0 ){
                //std::cout << "RESET TO CURRENT TIME" << std::endl;
                resetToCurrentTime();
            }
        }

        //SET SPEED OF TIME
        if( strncmp( receivedChars, "speed", 5 ) == 0 ){
            // Parse string to int
            int tmpVal = atoi(receivedChars + 6);
            timeSpeed.setVal( static_cast<int>(tmpVal) );
            //std::cout << "Speed of time: " << timeSpeed.getVal() << std::endl;
        }

        //SET DATE MANUALLY
        if( strncmp( receivedChars, "date", 4 ) == 0 ){
            //std::cout << "SET DATE MANUALLY" << std::endl;
            std::string tempTime = ( receivedChars + 5 );

            std::string tempYear = tempTime.substr(0,4);
            std::string tempMonth = tempTime.substr(5,2);
            std::string tempDay = tempTime.substr(8,2);
            std::string tempHour = tempTime.substr(11,2);
            std::string tempMinute = tempTime.substr(14,2);
            std::string tempSeconds = tempTime.substr(17,2);

            currentTime[YEAR] = atoi( tempYear.c_str() );
            currentTime[MONTH] = atoi( tempMonth.c_str() );
            currentTime[DAY] = atoi( tempDay.c_str() );
            currentTime[HOUR] = atoi( tempHour.c_str() );
            currentTime[MINUTE] = atoi( tempMinute.c_str() );
            currentTime[SECOND] = atoi( tempSeconds.c_str() );
        }
        sgct::MessageHandler::instance()->print("Message: '%s', size: %d\n", receivedChars, size);
    }
}

void externalControlStatusCallback( bool connected ){
    if( connected )
        sgct::MessageHandler::instance()->print("External control connected.\n");
    else
        sgct::MessageHandler::instance()->print("External control disconnected.\n");
}


/*
 http://en.cppreference.com/w/cpp/chrono/c/strftime
 Function to calculate the current time, maybe needed to send this out to all the slaves later?
 */
void resetToCurrentTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buffer[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buffer, sizeof(buffer), "%F-%X", &tstruct);

/*
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S");
	std::string tempTime = ss.str();
*/
    std::string tempTime(&buffer[0]);

    std::string tempYear = tempTime.substr(0,4);
    std::string tempMonth= tempTime.substr(5,2);
    std::string tempDay = tempTime.substr(8,2);
    std::string tempHour= tempTime.substr(11,2);
    std::string tempMinute= tempTime.substr(14,2);
    std::string tempSeconds= tempTime.substr(17,2);

    currentTime[YEAR] = atoi(tempYear.c_str());
    currentTime[MONTH] = atoi(tempMonth.c_str());
    currentTime[DAY] = atoi(tempDay.c_str());
    currentTime[HOUR] = atoi(tempHour.c_str());
    currentTime[MINUTE] = atoi(tempMinute.c_str());
    currentTime[SECOND] = atoi(tempSeconds.c_str());

}

/*Function to calculate the suns illumination angle relative to the earth*/
void calcSunPosition(){
    SpiceDouble r = 6371.0;         // Earth radius [km]
    SpiceDouble ourLon = 16.192421;    // Longitude of Nrkpg
    SpiceDouble ourLat = 58.587745;    // Latitude of Nrkpg

    SpiceChar *abcorr;
    SpiceChar *obsrvr;
    SpiceChar *target;
    SpiceChar *ref;

    SpiceDouble ourPosition[3];
    SpiceDouble sunPointOnEarth[3];
    SpiceDouble sunPosition[3];

    SpiceDouble et, lt;
    SpiceDouble srfvec[3];
    SpiceDouble trgepc;
    SpiceDouble angle;

    SpiceDouble solar;
    SpiceDouble emissn;
    SpiceDouble sslemi;
    SpiceDouble sslphs;
    SpiceDouble sslsol;
    SpiceDouble ssolpt[3];
    SpiceDouble phase;
    SpiceDouble emission;

    //convert planetocentric r/lon/lat to Cartesian 3-vector

    ourLon = ourLon * rpd_c();
    ourLat = ourLat * rpd_c();

    latrec_c( r, ourLon, ourLat, ourPosition );

    std::string tempDate = std::to_string( currentTime[YEAR] ) + " " + std::to_string( currentTime[MONTH] ) + " " + std::to_string( currentTime[DAY] ) + " " + std::to_string( currentTime[HOUR] )  + ":" + std::to_string( currentTime[MINUTE] ) + ":" + std::to_string( currentTime[SECOND] );

    char *cstr = new char[tempDate.length() + 1];
    strcpy(cstr, tempDate.c_str());

    SpiceChar * date = cstr;

    //Used to convert between time as a string into ET, which is in seconds.
    str2et_c ( date, &et );

    delete [] cstr;

    target = "EARTH";
    obsrvr = "SUN";
    abcorr = "LT+S";
    ref = "iau_earth";

    //Calculate Zenit point on earth
    subslr_c ( "Near point: ellipsoid", target, et, ref, abcorr, obsrvr, sunPointOnEarth, &trgepc, srfvec );

    //Calculate suns emission angle
    //ilumin_c ( "Ellipsoid", target, et, ref, abcorr, obsrvr, ourPosition, &trgepc, srfvec, &phase, &solar, &emission );

    //fSunAnglePhi = 3.1415/2 - emission;

    SpiceDouble sunPointLon = 0;    // Longitude of zenit
    SpiceDouble sunPointLat = 0;    // Latitude of zenit

    reclat_c(&sunPointOnEarth, &r, &sunPointLon, &sunPointLat);

    fSunAnglePhi = 3.1415/2 - (ourLat-sunPointLat);

    fSunAngleTheta = ourLon - sunPointLon;

}

void addSecondToTime(){
    for(int i = 0; i < timeSpeed.getVal(); i++){
        bool leapYear = false;
        if ( ( (currentTime[YEAR] % 4 == 0) && (currentTime[YEAR] % 100 != 0) ) || (currentTime[YEAR] % 400 == 0) ){
            leapYear = true;
        }

        //Add Second
        currentTime[SECOND] += 1;

        //Add Minute
        if ( currentTime[SECOND] >= 60 ){
            currentTime[MINUTE] += 1;
            currentTime[SECOND] = 0;
        }

        //Add Hour
        if ( currentTime[MINUTE] >= 60 ){
            currentTime[HOUR] += 1;
            currentTime[MINUTE] = 0;
        }

        //Add Day
        if ( currentTime[HOUR] >= 24 ){
            currentTime[DAY] += 1;
            currentTime[HOUR] = 0;
        }

        //Add Month
            //February and leap year
            if ( leapYear && currentTime[MONTH] == 2 && currentTime[DAY] > 29 ){
                currentTime[MONTH] += 1;
                currentTime[DAY] = 1;
            }

            else if ( currentTime[MONTH] == 2 && currentTime[DAY] > 28 )
            {
                currentTime[MONTH] += 1;
                currentTime[DAY] = 1;
            }

            else if( (currentTime[MONTH] == 4 || currentTime[MONTH] == 6 || currentTime[MONTH] == 9 ||
                      currentTime[MONTH] == 11) && currentTime[DAY] > 30 ){
                currentTime[MONTH] += 1;
                currentTime[DAY] = 1;
            }

            else if( currentTime[DAY] > 31 ){
                currentTime[MONTH] += 1;
                currentTime[DAY] = 1;
            }

        //Add Year
        if ( currentTime[MONTH] > 12 ) {
            currentTime[YEAR] += 1;
            currentTime[MONTH] = 1;
        }
    }
}

//Ska skrivas om...
void calcSkyColor(float fSunAnglePhi,float &fAmb, glm::vec4 &sColor){

    float fSine = sin(fSunAnglePhi);

    // We'll change color of skies depending on sun's position
    gEngine->setClearColor(std::max(0.0f, 0.3f*fSine), std::max(0.0f, 0.9f*fSine), std::max(0.0f, 0.9f*fSine), 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(fSunAnglePhi >= 30.0f*3.1415/180.0 && fSunAnglePhi <= 150.0f*3.1415/180.0) //DAY
    {
        sColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        fAmb = 0.8f;
    }
    else if(fSunAnglePhi <= 0.0f*3.1415/180.0 || fSunAnglePhi >= 180.0f*3.1415/180.0) //NIGHT
    {
        sColor = glm::vec4(110.0f/256.0f, 40.0f/256.0f, 189.0f/256.0f, 1.0f);
        fAmb = 0.3f;
    }
    else // DAWN/DUSK
    {
        sColor = glm::vec4(247.0f/256.0f, 21.0f/256.0f, 21.0f/256.0f, 1.0f);
        fAmb = 0.6f;
    }

}
