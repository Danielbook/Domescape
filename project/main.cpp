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
//#include </home/adam/Dokument/GitHub/CSPICE/cspice/include/SpiceUsr.h>
#include <SpiceZfc.h>
//#include </home/adam/Dokument/GitHub/CSPICE/cspice/include/SpiceZfc.h>

#include "model.hpp"

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
const std::string currentDateTime(); // Used to calculate the time of the current computer
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

/*------------------SHADER------------------*/
sgct::ShaderProgram mSp;
GLint myTextureLocations[]	= { -1, -1 };
GLint MVP_Loc_G = -1;
GLint MV_Loc_G = -1;
GLint MVL_Loc_G = -1;
GLint NM_Loc_G = -1;
GLint lDir_Loc_G = -1;
GLint Amb_Loc_G = -1;

GLint MVP_Loc = -1;
GLint NM_Loc = -1;
GLint sColor_Loc = -1;
GLint lDir_Loc = -1;
GLint Amb_Loc = -1;
GLint Tex_Loc = -1;
/*------------------------------------------*/

/*------------------ORIENTATION------------------*/
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

sgct::SharedBool showStats(false);
sgct::SharedBool showGraph(false);
/*---------------------------------------*/

float sunAngle;

//Skapa sky, sun, moon. Kolla Demo. Sen är det bara att ritaut dem där nere, skissa med papper och penna!
model land;
model box;
model sun;

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


#ifdef __APPLE__
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) )
    {
        delete gEngine;
        return EXIT_FAILURE;
    }
#endif

//#ifdef __MSC_VER__
    if( !gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile ) )
    {
        delete gEngine;
        return EXIT_FAILURE;
    }
//#endif

#ifdef __linux__
    if( !gEngine->init( ) )
    {
        delete gEngine;
        return EXIT_FAILURE;
    }
#endif

    //TEMPORARY
    sunAngle = calcSunPosition();

    // Main loop
    gEngine->render();

    // Clean up
    delete gEngine;

    // Exit program
    exit( EXIT_SUCCESS );

    return( 0 );
}

void myInitOGLFun()
{
    sgct::TextureManager::instance()->setWarpingMode(GL_REPEAT, GL_REPEAT);
    sgct::TextureManager::instance()->setAnisotropicFilterSize(4.0f);
    sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);

    if (glGenVertexArrays == NULL)
    {
        printf("THIS IS THE PROBLEM");
    }
    
    initHeightMap();

    //Set up backface culling
    glCullFace(GL_BACK);

    //Textures
    glEnable(GL_TEXTURE_2D);
    sgct::TextureManager::instance()->loadTexure("box", "texture/box.png", true);
    box.readOBJ("mesh/box.obj");

    sgct::TextureManager::instance()->loadTexure("sun", "texture/sun.jpg", true);
    sun.createSphere(50.0f, 80);


    //ObjReader  objReader("mesh/cornell_box.obj");

    //Initialize Shader Xform (simple)
    sgct::ShaderManager::instance()->addShaderProgram( "xform", "simple.vert", "simple.frag" );
    sgct::ShaderManager::instance()->bindShaderProgram( "xform" );

    MVP_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "MVP" );
    NM_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "NM" );
    sColor_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "sunColor" );
    lDir_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "lightDir" );
    Amb_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "fAmbInt" );
    Tex_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "Tex" );
    glUniform1i( Tex_Loc, 0 );


    sgct::ShaderManager::instance()->unBindShaderProgram();
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


        sunX -= 1.0f;

        if(sunX < -500.0f){
            sunX = 500.0f;
            sunY = 100.0f;
        }

        if(sunX>0)
        {
            sunY += 1.0f;
        }
        else{
            sunY -= 1.0f;
        }
        sunPosition = glm::vec3(sunX,sunY,0.0f);

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

void myPostSyncPreDrawFun()
{
    //GUI
    gEngine->setDisplayInfoVisibility( showStats.getVal() );
    //
    
    if( reloadShader.getVal() )
    {
        sgct::ShaderProgram sp = sgct::ShaderManager::instance()->getShaderProgram( "xform" );
        sp.reload();

        //reset locations
        sp.bind();

        MVP_Loc = sp.getUniformLocation( "MVP" );
        NM_Loc = sp.getUniformLocation( "NM" );
        Tex_Loc = sp.getUniformLocation( "Tex" );
        sColor_Loc = sp.getUniformLocation("sunColor");
        lDir_Loc = sp.getUniformLocation("lightDir");
        Amb_Loc = sp.getUniformLocation("fAmbInt");
        glUniform1i( Tex_Loc, 0 );

        sp.unbind();
        reloadShader.setVal(false);
    }
}

void myDrawFun()
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    
    //create scene transform (animation)
    glm::mat4 scene_mat = xform.getVal();

    //Projection matrix - för att ändra clipping
    glm::mat4 Model = glm::mat4(1.0f);
    //glm::mat4 P = glm::infinitePerspective(50.0f, 16.0f/9.0f, 0.1f);
    glm::mat4 P = glm::perspectiveFov(50.0f, 1200.0f, 720.0f, 0.1f, 1000.0f);

    glm::mat4 MV = gEngine->getActiveModelViewMatrix() * scene_mat;
    glm::mat4 MV_light = gEngine->getActiveModelViewMatrix();
    glm::mat3 NML = glm::inverseTranspose(glm::mat3( MV_light ));

    glm::mat4 MVP = P * MV * Model;
    //glm::mat4 MVP = gEngine->getActiveModelViewProjectionMatrix() * scene_mat;
    glm::mat3 NM = glm::inverseTranspose(glm::mat3( MV ));

    //Call calcSunPosition();
    //Ex: vec3 sunData(float fSunDis, float fSunAngleTheta, float fSunAnglePhi) = calcSunPosition();
    
    // Set light properties
    float fSunDis = 70;
    //float fSunAngleTheta = 45.0f * 3.1415/180.0; // Degrees Celsius to radians
    float fSunAngleTheta = calcSunPosition();
    std::cout << "Sun angle: " << fSunAngleTheta << std::endl;
    float fSunAnglePhi = 20.0f * 3.1415/180.0; //Degrees Celsius to radians
    float fSine = sin(fSunAnglePhi);
    glm::vec3 vSunPos(fSunDis*sin(fSunAngleTheta)*cos(fSunAnglePhi),fSunDis*sin(fSunAngleTheta)*sin(fSunAnglePhi),fSunDis*cos(fSunAngleTheta));

    // We'll change color of skies depending on sun's position
    glClearColor(std::max(0.0f, 0.3f*fSine), std::max(0.0f, 0.9f*fSine), std::max(0.0f, 0.9f*fSine), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec4 sColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //Calculate Sun Color depending on sunAngle!
    glm::vec3 lDir = glm::normalize(vSunPos);
    float fAmb = 0.3f; //Calculate Ambient Light depending on sunAngle!

    drawHeightMap(MVP, NML, MV, MV_light, lDir, fAmb);

    //Bind Shader
    sgct::ShaderManager::instance()->bindShaderProgram( "xform" );

    glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix3fv(NM_Loc, 1, GL_FALSE, &NM[0][0]);
    glUniform4fv(sColor_Loc, 1, &sColor[0]);
    glUniform3fv(lDir_Loc, 1, &lDir[0]);
    glUniform1fv(Amb_Loc, 1, &fAmb);

    //BOX
    glm::mat4 NyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!
        NyMVP = glm::translate(NyMVP, glm::vec3(0.0f, 0.0f, -5.0f));
        NyMVP = glm::scale(NyMVP, glm::vec3(2.0f, 2.0f, 2.0f));

        //Send the transformations, texture and render
        glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, glm::value_ptr(NyMVP));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("box"));
        box.render();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //SUN
    NyMVP = MVP;
        //Transformations from origo. ORDER MATTERS!
        NyMVP = glm::translate(NyMVP, sunPosition);

        //Send the transformations, texture and render
        glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, glm::value_ptr(NyMVP));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureId("sun"));
        sun.render();

    sgct::ShaderManager::instance()->unBindShaderProgram();

    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );

}

void myEncodeFun()
{
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
    sgct::SharedData::instance()->writeBool( &showStats );
}

void myDecodeFun()
{
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
    sgct::SharedData::instance()->readBool( &showStats );

}

/*!
	De-allocate data from GPU
	Textures are deleted automatically when using texture manager
	Shaders are deleted automatically when using shader manager
 */
void myCleanUpFun()
{
    if(vertexPositionBuffer)
        glDeleteBuffers(1, &vertexPositionBuffer);
    if(texCoordBuffer)
        glDeleteBuffers(1, &texCoordBuffer);
    if(vertexArray)
        glDeleteVertexArrays(1, &vertexArray);
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

            case SGCT_KEY_W:
                dirButtons[FORWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

            case SGCT_KEY_S:
                dirButtons[BACKWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

            case SGCT_KEY_A:
                dirButtons[LEFT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

            case SGCT_KEY_D:
                dirButtons[RIGHT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

            case SGCT_KEY_UP:
                dirButtons[FORWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

            case SGCT_KEY_DOWN:
                dirButtons[BACKWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

            case SGCT_KEY_LEFT:
                dirButtons[LEFT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

            case SGCT_KEY_RIGHT:
                dirButtons[RIGHT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

                //Running
            case SGCT_KEY_LEFT_SHIFT:
            case SGCT_KEY_RIGHT_SHIFT:
                runningButton = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
                break;

        	case SGCT_KEY_Q:
            	dirButtons[UP] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
				break;

        	case SGCT_KEY_E:
	            dirButtons[DOWN] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
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
                //set refPos
                sgct::Engine::getMousePos(gEngine->getFocusedWindowIndex(), &mouseXPos[1], &mouseYPos[1]);
                break;
        }
    }
}


void externalControlMessageCallback(const char * receivedChars, int size)
{
    if( gEngine->isMaster() )
    {
        if(size == 7 && strncmp(receivedChars, "stats", 5) == 0)
        {
            showStats.setVal(strncmp(receivedChars + 6, "1", 1) == 0);
        }
        else if(size == 7 && strncmp(receivedChars, "graph", 5) == 0)
        {
            showGraph.setVal(strncmp(receivedChars + 6, "1", 1) == 0);
        }
        
        sgct::MessageHandler::instance()->print("Message: '%s', size: %d\n", receivedChars, size);
    }
}

void externalControlStatusCallback(bool connected)
{
    if(connected)
        sgct::MessageHandler::instance()->print("External control connected.\n");
    else
        sgct::MessageHandler::instance()->print("External control disconnected.\n");
}


//REGULAR FUNCTIONS
void initHeightMap()
{
    //setup textures
    sgct::TextureManager::instance()->loadTexure("heightmap", "texture/map.png", true, 0);
    sgct::TextureManager::instance()->loadTexure("normalmap", "texture/normal.png", true, 0);

    //setup shader
    sgct::ShaderManager::instance()->addShaderProgram( mSp, "Heightmap", "heightmap.vert", "heightmap.frag" );

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

const std::string currentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buffer[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buffer, sizeof(buffer), "%Y %b %d %X", &tstruct);
    
    std::cout << buffer << std:: endl;
    
    return buffer;
}


/*Function to calculate the suns illumination angle relative to the earth*/
float calcSunPosition(){

    SpiceDouble r = 6371.0;
    SpiceDouble lon = 16.1833333;
    SpiceDouble lat = 58.6;

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
    
    std::string str = currentDateTime();
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    
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
    
    std::cout << "Our position on earth: " << ourPosition[0] << ", " << ourPosition[1] << ", " << ourPosition[2] << std::endl;
    std::cout << "Suns position relative to earth: " << sunPosition[0] << ", " << sunPosition[1] << ", " << sunPosition[2] << std::endl;
    std::cout << "Suns point on earth (Zenit): " << sunPointOnEarth[0] << ", " << sunPointOnEarth[1] << ", " << sunPointOnEarth[2] << std::endl;
    
    float a, b, xd, yd, zd;
    
    //CALCULATE DISTANCE BETWEEN ZENIT POINT AND SUN = a
    xd = sunPosition[0]-sunPointOnEarth[0];
    yd = sunPosition[1]-sunPointOnEarth[1];
    zd = sunPosition[2]-sunPointOnEarth[2];
    a = sqrtf(xd*xd + yd*yd + zd*zd);
    
    //CALCULATE DISTANCE BETWEEN US AND THE ZENIT POINT = b
    xd = ourPosition[0]-sunPointOnEarth[0];
    yd = ourPosition[1]-sunPointOnEarth[1];
    zd = ourPosition[2]-sunPointOnEarth[2];
    b = sqrtf(xd*xd + yd*yd + zd*zd);

    //CALCULATE ANGLE ( arctan (a/b) = angle
    angle = atan(a/b);
    
    std::cout << "Sun angle in radians: " << angle << std::endl;
    
    //Convert the angles to degrees
    //angle *= dpr_c();
    
    return angle;
}
