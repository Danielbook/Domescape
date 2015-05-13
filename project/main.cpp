#include "sgct.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <string>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

//For the time function
#include <time.h>

#include <SpiceUsr.h>
#include <SpiceZfc.h>

#include "model.hpp"
//#include "objloader.hpp"
//#include "MVstack.hpp"


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


/*------------------MOVEMENT------------------*/
float rotationSpeed = 0.1f;
float walkingSpeed = 2.5f;
float runningSpeed = 5.0f;
/*--------------------------------------------*/

/*------------------REGULAR FUNCTIONS------------------*/
float calcSunPosition(); // Calculates the suns position
void resetToCurrentTime(); // Used to calculate the time of the current computer
/*-----------------------------------------------------*/

/*------------------HEIGHTMAP------------------*/
void generateTerrainGrid( float width, float height, unsigned int wRes, unsigned int dRes );
void initHeightMap();
void drawHeightMap(glm::mat4 MVP, glm::mat3 NM, glm::mat4 MV, glm::mat4 MV_light, glm::vec3 lDir, float fAmb);

//opengl objects
GLuint vertexArray = GL_FALSE;
GLuint vertexPositionBuffer = GL_FALSE;
GLuint texCoordBuffer = GL_FALSE;

//variables to share across cluster
sgct::SharedBool wireframe(false);
sgct::SharedBool info(false);
sgct::SharedBool stats(false);
sgct::SharedBool takeScreenshot(false);
sgct::SharedBool useTracking(false);
sgct::SharedInt stereoMode(0);

//geometry
std::vector<float> mVertPos;
std::vector<float> mTexCoord;
GLsizei mNumberOfVerts = 0;
/*---------------------------------------------*/

/*------------------SHADERS------------------*/
//Heightmap shader
sgct::ShaderProgram mSp;
GLint myTextureLocations[]	= { -1, -1 };
GLint MVP_Loc_G = -1;
GLint MV_Loc_G = -1;
GLint MVL_Loc_G = -1;
GLint NM_Loc_G = -1;
GLint lDir_Loc_G = -1;
GLint Amb_Loc_G = -1;

//Shader Shadowmap
GLint depthMVP_Loc = -1;
GLint depthBiasMVP_Loc = -1;
GLint shadowmap_Loc = -1;
GLuint FramebufferName = 0;
GLuint depthTexture;

//Shader Scene
GLint MVP_Loc = -1;
GLint NM_Loc = -1;
GLint sColor_Loc = -1;
GLint lDir_Loc = -1;
GLint Amb_Loc = -1;
GLint Tex_Loc = -1;

//Shader Sky
GLint MVP_Loc_S = -1;
GLint NM_Loc_S = -1;
GLint lDir_Loc_S = -1;
GLint Tex_Loc_S = -1;
GLint Glow_Loc_S = -1;
GLint SunColor_Loc_S = -1;
/*------------------------------------------*/

//Oriantation variables
bool dirButtons[6];
enum directions { FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN };

bool runningButton = false;
bool mouseLeftButton = false;

double mouseDx = 0.0;
double mouseDy = 0.0;

double mouseXPos[] = { 0.0, 0.0 };
double mouseYPos[] = { 0.0, 0.0 };

glm::vec3 bView(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);
glm::vec3 pos(0.0f, 0.0f, 0.0f);


float sunX = 500.0f;
float sunY = 100.f;
glm::vec3 sunPosition(sunX, sunY, 0.0f);
/*-----------------------------------------------*/

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
sgct::SharedBool writeOut = false;
/*---------------------------------------*/

void addSecondToTime();

enum timeVariables{YEAR = 0, MONTH = 1, DAY = 2, HOUR = 3, MINUTE = 4, SECOND = 5};
int currentTime[6];

int timeCount = 0;

float sunAngle;

model landscape;
model box;
model sun;
model skyDome;

int main( int argc, char* argv[] )
{
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

    /*------------------SPICE------------------*/
    //load kernels
    furnsh_c( "kernels/naif0011.tls" ); //Is a generic kernel that you can use to get the positions of Earth and the Sun for various times
    furnsh_c( "kernels/de430.bsp" ); //Is a leapsecond kernel so that you get the accurate times
    furnsh_c( "kernels/pck00010.tpc" ); //Might also be needed
    /*-----------------------------------------*/

    for(int i=0; i<6; i++)
        dirButtons[i] = false;


#if __APPLE__
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) )
    {
        delete gEngine;
        return EXIT_FAILURE;
    }

#elif __MSC_VER__
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) )
    {
        delete gEngine;
        return EXIT_FAILURE;
    }

#elif __WIN32__
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) )
    {
        delete gEngine;
        return EXIT_FAILURE;
    }

#elif __linux__
    if( !gEngine->init( ) )
    {
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
    sgct::TextureManager::instance()->setAnisotropicFilterSize(4.0f);
    sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);

    // TO BE REPLACED
    //initHeightMap();

    /*----------------OBJECTS AND TEXTURES--------------*/

    glEnable(GL_TEXTURE_2D);
    sgct::TextureManager::instance()->loadTexure("box", "texture/box.png", true);
    box.readOBJ("mesh/box.obj");

    sgct::TextureManager::instance()->loadTexure("sun", "texture/sun.jpg", true);
    sun.createSphere(10.0f, 80);

    sgct::TextureManager::instance()->loadTexure("landscape", "texture/landscape2.png", true);
    landscape.readOBJ("mesh/landscape2.obj");

    skyDome.createSphere(5.0f, 100);

    /*-----------------------------------------------------*/

        /*---------------------SHADOWMAP-------------*/
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
     glGenFramebuffers(1, &FramebufferName);
     glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

     // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
     glGenTextures(1, &depthTexture);
     glBindTexture(GL_TEXTURE_2D, depthTexture);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

     glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

     glDrawBuffer(GL_NONE); // No color buffer is drawn to.

     // Always check that our framebuffer is ok
     if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
     { std::cout << "Frame Buffer in bad state" << std::endl;  }

    /*-----------------------------------------------------------*/


    /*---------------------SHADERS-----------------------*/

    //Initialize Shader scene (simple)
    sgct::ShaderManager::instance()->addShaderProgram( "scene", "shaders/simple.vert", "shaders/simple.frag" );
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
    glUniform1i( shadowmap_Loc, 0 );

    sgct::ShaderManager::instance()->unBindShaderProgram();


    //Initialize Shader shadowmap (shadow)
    sgct::ShaderManager::instance()->addShaderProgram( "shadowmap", "shaders/shadow.vert", "shaders/shadow.frag" );
    sgct::ShaderManager::instance()->bindShaderProgram( "shadowmap" );

    depthMVP_Loc = sgct::ShaderManager::instance()->getShaderProgram( "shadowmap").getUniformLocation( "depthMVP" );

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
    glUniform1i( Glow_Loc_S, 2 );
    glUniform1i( SunColor_Loc_S, 1 );
    glUniform1i( Tex_Loc_S, 0 );

    sgct::ShaderManager::instance()->unBindShaderProgram();

    /*---------------------------------------------------------*/
}

void myPreSyncFun(){
    if( gEngine->isMaster() ){
        curr_time.setVal( sgct::Engine::getTime() ); //Används ej för tillfället?

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


        //SUNPOSITION, fullösning
        sunX -= 1.0f;

        if(sunX < -500.0f){
            sunX = 500.0f;
            sunY = 100.0f;
        }

        if(sunX>0){
            sunY += 1.0f;
        }
        else{
            sunY -= 1.0f;
        }
        sunPosition = glm::vec3(sunX,sunY,0.0f);


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
    if( timeIsTicking.getVal() == true && writeOut.getVal() == true){
        std::cout << "Time is ticking" << std::endl;
    }

    else if( timeIsTicking.getVal() == false && writeOut.getVal() == true ){
        std::cout << "Time is paused" << std::endl;
    }

    if( reloadShader.getVal() )
    {
        sgct::ShaderProgram sp = sgct::ShaderManager::instance()->getShaderProgram( "scene" );
        sp.reload();

        //reset locations
        sp.bind();

        MVP_Loc = sp.getUniformLocation( "MVP" );
        NM_Loc = sp.getUniformLocation( "NM" );
        depthMVP_Loc = sp.getUniformLocation( "depthMVP" );
        sColor_Loc = sp.getUniformLocation("sunColor");
        lDir_Loc = sp.getUniformLocation("lightDir");
        Amb_Loc = sp.getUniformLocation("fAmbInt");
        Tex_Loc = sp.getUniformLocation( "Tex" );
        glUniform1i( Tex_Loc, 0 );
        shadowmap_Loc = sp.getUniformLocation( "shadowMap" );
        glUniform1i( shadowmap_Loc, 1);

        sp.unbind();
        reloadShader.setVal(false);


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
}

void myDrawFun(){
    ////fuLhaxX
        writeOut = false;
        if( timeIsTicking.getVal() == true ){
            timeCount++;
        }

        if(timeCount == 60){
            writeOut.setVal(true);
            timeCount = 0;
        }


    if(writeOut.getVal())
    {
        if( timeIsTicking.getVal() ){
            for(int i = 0; i < timeSpeed.getVal(); i++){
                addSecondToTime();
            }
        }

    std::cout << currentTime[YEAR] << " " << currentTime[MONTH] << " " << currentTime[DAY] << " " << currentTime[HOUR] << ":" << currentTime[MINUTE] << ":" << currentTime[SECOND] << std::endl;
    }
    /////

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //create scene transform (animation)
    glm::mat4 scene_mat = xform.getVal();

    //För heightmap - onödiga?
    glm::mat4 MV = gEngine->getActiveModelViewMatrix() * scene_mat;
    glm::mat4 MV_light = gEngine->getActiveModelViewMatrix();
    glm::mat3 NML = glm::inverseTranspose(glm::mat3( MV_light ));

    gEngine->setNearAndFarClippingPlanes(0.1f, 2000.0f);
    glm::mat4 MVP = gEngine->getActiveModelViewProjectionMatrix() * scene_mat;
    glm::mat3 NM = glm::inverseTranspose(glm::mat3( MV ));


    // Set light properties
    float fSunDis = 800;
    float fSunAnglePhi = calcSunPosition(); //SunAngle in radians

    float fSunAngleTheta = 20.0f * 3.1415/180.0; //Degrees Celsius to radians -> Bero på månad!?
    float fSine = sin(fSunAnglePhi);
    glm::vec3 vSunPos(fSunDis*sin(fSunAngleTheta)*cos(fSunAnglePhi),fSunDis*sin(fSunAngleTheta)*sin(fSunAnglePhi),fSunDis*cos(fSunAngleTheta));

    // We'll change color of skies depending on sun's position
    glClearColor(std::max(0.0f, 0.3f*fSine), std::max(0.0f, 0.9f*fSine), std::max(0.0f, 0.9f*fSine), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Calculate Sun color and Ambient light
    float fAmb = 0.2f; //Initialize to low for debugging purposes
    glm::vec4 sColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f); //Initialize to low for debugging purposes
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

    glm::vec3 lDir = glm::normalize(vSunPos);

    //TO BE REPLACED
    //drawHeightMap(MVP, NML, MV, MV_light, lDir, fAmb);



    /*------------------SHADOW MAP------------------*/
/*    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glViewport(0,0,1024,1024); // Render on the whole framebuffer

    glm::vec3 lightInvDir = lDir;

    // Compute the MVP matrix from the light's point of view
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-100,100,-100,100,-100,200);
    glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 depthModelMatrix = glm::mat4(1.0);
    glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,    0.0, 0.5, 0.0, 0.0,    0.0, 0.0, 0.5, 0.0,    0.5, 0.5, 0.5, 1.0);
    glm::mat4 depthBiasMVP = biasMatrix*depthMVP;

    sgct::ShaderManager::instance()->bindShaderProgram( "shadowmap" );

    glUniformMatrix4fv(depthMVP_Loc, 1, GL_FALSE, &depthMVP[0][0]);

    // HA MED ALLA TRANSFORMATIONER? MÅSTE STÄMMA ÖVERENS MED SCENE-SHADER
    glm::mat4 nyDepthMVP = depthMVP;
    nyDepthMVP = glm::translate(nyDepthMVP, glm::vec3(0.0f, -20.0f, 0.0f));
    nyDepthMVP = glm::scale(nyDepthMVP, glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(depthMVP_Loc, 1, GL_FALSE, glm::value_ptr(nyDepthMVP));
    landscape.render();

    nyDepthMVP = depthMVP;
    nyDepthMVP = glm::translate(nyDepthMVP, glm::vec3(0.0f, 0.0f, -5.0f));
    nyDepthMVP = glm::scale(nyDepthMVP, glm::vec3(2.0f, 2.0f, 2.0f));
    glUniformMatrix4fv(depthMVP_Loc, 1, GL_FALSE, glm::value_ptr(nyDepthMVP));
    box.render();

    sgct::ShaderManager::instance()->unBindShaderProgram();

    // Render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,1024,1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // Clear the screen
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
*/
    /*------------------------------------------------*/

    /*------------------SCENE SHADER------------------*/

    //Bind Shader scene
    sgct::ShaderManager::instance()->bindShaderProgram( "scene" );

    glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix3fv(NM_Loc, 1, GL_FALSE, &NM[0][0]);
    glUniform4fv(sColor_Loc, 1, &sColor[0]);
    glUniform3fv(lDir_Loc, 1, &lDir[0]);
    glUniform1fv(Amb_Loc, 1, &fAmb);
    //glUniformMatrix4fv(depthBiasMVP_Loc, 1, GL_FALSE, &depthBiasMVP[0][0]);


    //LANDSCAPE
    glm::mat4 NyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!
        NyMVP = glm::translate(NyMVP, glm::vec3(0.0f, -20.0f, 0.0f));
        NyMVP = glm::scale(NyMVP, glm::vec3(1.0f, 1.0f, 1.0f));

        //Send the transformations, texture and render
        glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, glm::value_ptr(NyMVP));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("landscape"));
        glUniform1i(Tex_Loc, 0);
        //glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, depthTexture);
		//glUniform1i(shadowmap_Loc, 1);
        landscape.render();

    //BOX
    NyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!
        NyMVP = glm::translate(NyMVP, glm::vec3(0.0f, 0.0f, -5.0f));
        NyMVP = glm::scale(NyMVP, glm::vec3(2.0f, 2.0f, 2.0f));

        //Send the transformations, texture and render
        glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, glm::value_ptr(NyMVP));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("box"));
        glUniform1i(Tex_Loc, 0);
        //glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, depthTexture);
		//glUniform1i(shadowmap_Loc, 1);
        box.render();

    sgct::ShaderManager::instance()->unBindShaderProgram();

    /*----------------------------------------------*/

    /*------------------SKY SHADER------------------*/

    //Bind Shader sky
    sgct::ShaderManager::instance()->bindShaderProgram( "sky" );

    glUniformMatrix4fv(MVP_Loc_S, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix3fv(NM_Loc_S, 1, GL_FALSE, &NM[0][0]);
    glUniform3fv(lDir_Loc_S, 1, &lDir[0]);


    //SUN
    NyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!
        //NyMVP = glm::translate(NyMVP, sunPosition);
        NyMVP = glm::translate(NyMVP, vSunPos);

        //Send the transformations, texture and render
        glUniformMatrix4fv(MVP_Loc_S, 1, GL_FALSE, glm::value_ptr(NyMVP));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("sun"));
        glUniform1i(Tex_Loc, 0);
        sun.render();

/* SKIPPAR DENNA SÅ LÄNGE
    //SKYDOME
    NyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!

        //Send the transformations, texture and render
        glUniformMatrix4fv(MVP_Loc_S, 1, GL_FALSE, glm::value_ptr(NyMVP));
        //glBindTexture(GL_TEXTURE_2D, 0);
        //glUniform1i(Tex_Loc, 0);
        skyDome.render();
*/
    sgct::ShaderManager::instance()->unBindShaderProgram();

    /*----------------------------------------------*/

    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );

}

void myEncodeFun(){
    sgct::SharedData::instance()->writeObj( &xform );
    sgct::SharedData::instance()->writeDouble( &curr_time );
    sgct::SharedData::instance()->writeBool( &reloadShader );
    sgct::SharedData::instance()->writeBool( &wireframe );
    sgct::SharedData::instance()->writeBool( &info );
    sgct::SharedData::instance()->writeBool( &stats );
    sgct::SharedData::instance()->writeBool( &takeScreenshot );
    sgct::SharedData::instance()->writeBool( &useTracking );
    sgct::SharedData::instance()->writeInt( &stereoMode );

    //GUI
    sgct::SharedData::instance()->writeBool( &timeIsTicking );
    sgct::SharedData::instance()->writeString( &date );
    sgct::SharedData::instance()->writeInt( &timeSpeed );
    sgct::SharedData::instance()->writeBool( &writeOut );
}

void myDecodeFun(){
    sgct::SharedData::instance()->readObj( &xform );
    sgct::SharedData::instance()->readDouble( &curr_time );
    sgct::SharedData::instance()->readBool( &reloadShader );
    sgct::SharedData::instance()->readBool( &wireframe );
    sgct::SharedData::instance()->readBool( &info );
    sgct::SharedData::instance()->readBool( &stats );
    sgct::SharedData::instance()->readBool( &takeScreenshot );
    sgct::SharedData::instance()->readBool( &useTracking );
    sgct::SharedData::instance()->readInt( &stereoMode );

    //GUI
    sgct::SharedData::instance()->readBool( &timeIsTicking );
    sgct::SharedData::instance()->readString( &date );
    sgct::SharedData::instance()->readInt( &timeSpeed );
    sgct::SharedData::instance()->readBool( &writeOut );
}

/*!
	De-allocate data from GPU
	Textures are deleted automatically when using texture manager
	Shaders are deleted automatically when using shader manager
 */
void myCleanUpFun(){
    if(vertexPositionBuffer)
        glDeleteBuffers(1, &vertexPositionBuffer);
    if(texCoordBuffer)
        glDeleteBuffers(1, &texCoordBuffer);
    if(vertexArray)
        glDeleteVertexArrays(1, &vertexArray);

    	glDeleteFramebuffers(1, &FramebufferName);
    	glDeleteTextures(1, &depthTexture);
}


void keyCallback(int key, int action)
{
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
        if(size == 7 && strncmp(receivedChars, "pause", 5) == 0){
            if( strncmp(receivedChars, "pause=0", 7) == 0 ){
                timeIsTicking.setVal( true );
                //std::cout << "CONTINUE TIME" << std::endl;
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
            //parse string to int
            int tmpVal = atoi(receivedChars + 6);

            timeSpeed.setVal(static_cast<int>(tmpVal));

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

            currentTime[YEAR] = atoi(tempYear.c_str());
            currentTime[MONTH] = atoi(tempMonth.c_str());
            currentTime[DAY] = atoi(tempDay.c_str());
            currentTime[HOUR] = atoi(tempHour.c_str());
            currentTime[MINUTE] = atoi(tempMinute.c_str());
            currentTime[SECOND] = atoi(tempSeconds.c_str());

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

void initHeightMap(){
    //setup textures
    sgct::TextureManager::instance()->loadTexure("heightmap", "texture/map.png", true, 0);
    sgct::TextureManager::instance()->loadTexure("normalmap", "texture/normal.png", true, 0);

    //setup shader
    sgct::ShaderManager::instance()->addShaderProgram( mSp, "Heightmap", "shaders/heightmap.vert", "shaders/heightmap.frag" );

    mSp.bind();
    myTextureLocations[0]	= mSp.getUniformLocation( "hTex" );
    myTextureLocations[1]	= mSp.getUniformLocation( "nTex" );

    MVP_Loc_G		= mSp.getUniformLocation( "MVP" );
    MV_Loc_G		= mSp.getUniformLocation( "MV" );
    MVL_Loc_G		= mSp.getUniformLocation( "MV_light" );
    NM_Loc_G		= mSp.getUniformLocation( "normalMatrix" );
    lDir_Loc_G		= mSp.getUniformLocation( "light_dir" );
    Amb_Loc_G       = mSp.getUniformLocation( "lAmb");
    glUniform1i( myTextureLocations[0], 0 );
    glUniform1i( myTextureLocations[1], 1 );

    sgct::ShaderManager::instance()->unBindShaderProgram();

    //generate mesh
    generateTerrainGrid( 250.0f, 250.0f, 1024, 1024  );

    //generate vertex array
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    //generate vertex position buffer
    glGenBuffers(1, &vertexPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mVertPos.size(), &mVertPos[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          reinterpret_cast<void*>(0) // array buffer offset
                          );

    //generate texture coord buffer
    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mTexCoord.size(), &mTexCoord[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
                          1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          reinterpret_cast<void*>(0) // array buffer offset
                          );

    glBindVertexArray(0); //unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //cleanup
    mVertPos.clear();
    mTexCoord.clear();
}


void drawHeightMap(glm::mat4 MVP, glm::mat3 NM, glm::mat4 MV, glm::mat4 MV_light, glm::vec3 lDir, float fAmb)
{
    glEnable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId( "heightmap" ));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId( "normalmap" ));

    mSp.bind();

    glUniformMatrix4fv(MVP_Loc_G, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(MV_Loc_G, 1, GL_FALSE, &MV[0][0]);
    glUniformMatrix4fv(MVL_Loc_G, 1, GL_FALSE, &MV_light[0][0]);
    glUniformMatrix3fv(NM_Loc_G, 1, GL_FALSE, &NM[0][0]);
    glUniform3fv(lDir_Loc_G, 1, &lDir[0]);
    glUniform1fv(Amb_Loc_G, 1, &fAmb);

    glBindVertexArray(vertexArray);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLE_STRIP, 0, mNumberOfVerts);

    //unbind
    glBindVertexArray(0);

    mSp.unbind();
}

/*!
 Will draw a flat surface that can be used for the heightmapped terrain.
 @param	width	Width of the surface
 @param	depth	Depth of the surface
 @param	wRes	Width resolution of the surface
 @param	dRes	Depth resolution of the surface
 */
void generateTerrainGrid( float width, float depth, unsigned int wRes, unsigned int dRes )
{
    float wStart = -width * 0.5f;
    float dStart = -depth * 0.5f;


    float dW = width / static_cast<float>( wRes );
    float dD = depth / static_cast<float>( dRes );

    //cleanup
    mVertPos.clear();
    mTexCoord.clear();


    for( unsigned int depthIndex = 0; depthIndex < dRes; ++depthIndex )
    {
        float dPosLow = dStart + dD * static_cast<float>( depthIndex );
        float dPosHigh = dStart + dD * static_cast<float>( depthIndex + 1 );
        float dTexCoordLow = depthIndex / static_cast<float>( dRes );
        float dTexCoordHigh = (depthIndex+1) / static_cast<float>( dRes );

        for( unsigned widthIndex = 0; widthIndex < wRes; ++widthIndex )
        {
            float wPos = wStart + dW * static_cast<float>( widthIndex );
            float wTexCoord = widthIndex / static_cast<float>( wRes );

            //p0
            mVertPos.push_back( wPos ); //x
            mVertPos.push_back( 0.0f ); //y
            mVertPos.push_back( dPosLow ); //z

            //p1
            mVertPos.push_back( wPos ); //x
            mVertPos.push_back( 0.0f ); //y
            mVertPos.push_back( dPosHigh ); //z


            //tex0
            mTexCoord.push_back( wTexCoord ); //s
            mTexCoord.push_back( dTexCoordLow ); //t

            //tex1
            mTexCoord.push_back( wTexCoord ); //s
            mTexCoord.push_back( dTexCoordHigh ); //t
        }
    }

    mNumberOfVerts = static_cast<GLsizei>(mVertPos.size() / 3); //each vertex has three componets (x, y & z)
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

    //std::cout << buffer << std:: endl;
//    static_cast<int>()
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
float calcSunPosition(){

    SpiceDouble r = 6371.0;         // Earth radius [km]
    SpiceDouble lon = 16.192421;    // Longitude of Nrkpg
    SpiceDouble lat = 58.587745;    // Latitude of Nrkpg

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

    //#define   STRLEN    32
    //SpiceChar UTCDate[STRLEN];

    //Prompts the user to input date in format YEAR MONTH DAY HOUR:MIN:SEC
    //prompt_c("Date: ", STRLEN, UTCDate);

    //convert planetocentric r/lon/lat to Cartesian vector
    latrec_c( r, lon * rpd_c(), lat * rpd_c(), ourPosition );

//    std::string str = getCurrentTime();
//    char *cstr = new char[str.length() + 1];
//    strcpy(cstr, str.c_str());

    std::string tempDate = std::to_string( currentTime[YEAR] ) + " " + std::to_string( currentTime[MONTH] ) + " " + std::to_string( currentTime[DAY] ) + " " + std::to_string( currentTime[HOUR] )  + ":" + std::to_string( currentTime[MINUTE] ) + ":" + std::to_string( currentTime[SECOND] );

    char *cstr = new char[tempDate.length() + 1];
    strcpy(cstr, tempDate.c_str());

    SpiceChar * date = cstr;

    //Used to convert between time as a string into ET, which is in seconds.
    str2et_c ( date, &et ); /* <-- Denna ska vi kunna ändra på med en slider senare! */

    delete [] cstr;

    target = "EARTH";
    obsrvr = "SUN";
    abcorr = "LT+S";
    ref = "iau_earth";

    /*
     Provides you with the coordinates on the Earth where the Sun is directly above
              |-----------------------INPUT------------------------|  |---------OUTPUT-----------|
     subslr_c("method", "target", "et", "fixref", "abcorr", "obsrvr", "spoint", "trgepc", "srfvec");
     method: Is the name of the computation method, use "Intercept: ellipsoid"
     target: Is the name of the target body
     et:     Is the epoch, time stuff...
     fixref: Is the name of the body-fixed, body-centered reference frame associated with the target body
     abcorr: Is the aberration correction to be used, use "LT+S"
     obsrvr: Is the name of the observing body.  This is typically a spacecraft, the earth, or a surface point on the earth.
     spoint: Is a surface point on the target body, expressed in Cartesian coordinates
     trgepc: Is the "sub-solar point epoch."
     srfvec: Is the vector from the observer's position at `et' to the aberration-corrected (or optionally, geometric) position of `spoint'
            -srfvec is given in km
     ftp://naif.jpl.nasa.gov/pub/naif/toolkit_docs/FORTRAN/spicelib/subslr.html

               |----------------------------INPUT---------------------| |-----------OUTPUT------------|    */
    subslr_c ( "Intercept: ellipsoid", target, et, ref, abcorr, obsrvr, sunPointOnEarth, &trgepc, srfvec );

    /*
     Return the position of a target body relative to an observing
     body, optionally corrected for light time (planetary aberration)
     and stellar aberration.
     ftp://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/spkpos_c.html

             |------------INPUT-------------| |----OUTPUT-----|    */
    spkpos_c(target, et, ref, abcorr, obsrvr, sunPosition, &lt);

  /*  std::cout << "Our position on earth: " << ourPosition[0] << ", " << ourPosition[1] << ", " << ourPosition[2] << std::endl;
    std::cout << "Suns position relative to earth: " << sunPosition[0] << ", " << sunPosition[1] << ", " << sunPosition[2] << std::endl;
    std::cout << "Suns point on earth (Zenit): " << sunPointOnEarth[0] << ", " << sunPointOnEarth[1] << ", " << sunPointOnEarth[2] << std::endl;
*/
    float a, b, xd1, yd1, zd1, xd2, yd2, zd2;

    //Normalized vector from earth to sun (need to change sign?)
    SpiceDouble sunVec[3];
    SpiceDouble mag;
    unorm_c(sunPosition, sunVec, &mag);

    //CALCULATE DISTANCE BETWEEN US AND THE ZENIT POINT
    SpiceDouble posVecTemp[3];
    posVecTemp[0] = ourPosition[0]-sunPointOnEarth[0];
    posVecTemp[1] = ourPosition[1]-sunPointOnEarth[1];
    posVecTemp[2] = ourPosition[2]-sunPointOnEarth[2];

    SpiceDouble posVec[3];
    unorm_c(posVecTemp, posVec, &mag);


    //CALCULATE ANGLE
    angle = acos(vdot_c(posVec, sunVec));

    //std::cout << "Sun angle in radians: " << angle << std::endl;

    //Convert the angles to degrees
    //angle *= dpr_c();

    return angle;
}

void addSecondToTime(){
    bool leapYear = false;
    if ( ( (currentTime[YEAR] % 4 == 0) && (currentTime[YEAR] % 100 != 0) ) || (currentTime[YEAR] % 400 == 0) )
    {
        leapYear = true;
    }

    //Add Second
    currentTime[SECOND] += 1;

    //Add Minute
    if ( currentTime[SECOND] >= 60 ) {
        currentTime[MINUTE] += 1;
        currentTime[SECOND] = 0;
    }

    //Add Hour
    if ( currentTime[MINUTE] >= 60) {
        currentTime[HOUR] += 1;
        currentTime[MINUTE] = 0;
    }

    //Add Day
    if ( currentTime[HOUR] >= 24 ) {
        currentTime[DAY] += 1;
        currentTime[HOUR] = 0;
    }

    //Add Month
        //February and leap year
        if (leapYear && currentTime[MONTH] == 2 && currentTime[DAY] > 29) {
            currentTime[MONTH] += 1;
            currentTime[DAY] = 1;
        }

        else if (currentTime[MONTH] == 2 && currentTime[DAY] > 28)
        {
            currentTime[MONTH] += 1;
            currentTime[DAY] = 1;
        }

        else if( (currentTime[MONTH] == 4 || currentTime[MONTH] == 6 || currentTime[MONTH] == 9 ||
                  currentTime[MONTH] == 11) && currentTime[DAY] > 30  ){
            currentTime[MONTH] += 1;
            currentTime[DAY] = 1;
        }

        else if(currentTime[DAY] > 31){
            currentTime[MONTH] += 1;
            currentTime[DAY] = 1;
        }

    //Add Year
    if ( currentTime[MONTH] == 12 && currentTime[DAY] > 31 ) {
        currentTime[YEAR] += 1;
        currentTime[MONTH] = 1;
    }
}


