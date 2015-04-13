#include "sgct.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

sgct::Engine * gEngine;

void myDrawFun();
void myPreSyncFun();
void myInitOGLFun();
void myEncodeFun();
void myDecodeFun();
//input callbacks
void keyCallback(int key, int action);
void mouseButtonCallback(int button, int action);

void drawXZGrid(int size, float yPos);
void drawPyramid(float width);

//Loading .obj files
bool loadOBJ(const char * path,
             std::vector < glm::vec3 > & out_vertices,
             std::vector < glm::vec2 > & out_uvs,
             std::vector < glm::vec3 > & out_normals
             );

float rotationSpeed = 0.1f;
float walkingSpeed = 2.5f;
float runningSpeed = 5.0f;
float jumpingHeight = 10.0f;

glm::vec3 jumpingHeight3(0.0f, -0.1f, 0.0f);

GLuint myLandscapeDisplayList = 0;
const int landscapeSize = 200;
const int numberOfPyramids = 150;

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
	gEngine->setKeyboardCallbackFunction( keyCallback );
	gEngine->setMouseButtonCallbackFunction( mouseButtonCallback );
	gEngine->setClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	for(int i=0; i<4; i++)
		arrowButtons[i] = false;

	if( !gEngine->init() )
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	// Read our .obj file
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals; // Won't be used at the moment.

	sgct::SharedData::instance()->setEncodeFunction( myEncodeFun );
	sgct::SharedData::instance()->setDecodeFunction( myDecodeFun );

	// Main loop
	gEngine->render();

	// Clean up
	glDeleteLists(myLandscapeDisplayList, 1);
	delete gEngine;

	// Exit program
	exit( EXIT_SUCCESS );
}

void myInitOGLFun()
{
	//create and compile display list
	myLandscapeDisplayList = glGenLists(1);
	glNewList(myLandscapeDisplayList, GL_COMPILE);

	drawXZGrid(landscapeSize, -1.5f);

	//pick a seed for the random function (must be same on all nodes)
	srand(9745);
	for(int i=0; i<numberOfPyramids; i++)
	{
		float xPos = static_cast<float>(rand()%landscapeSize - landscapeSize/2);
		float zPos = static_cast<float>(rand()%landscapeSize - landscapeSize/2);

		glPushMatrix();
		glTranslatef(xPos, -1.5f, zPos);
		drawPyramid(0.6f);
		glPopMatrix();
	}

	glEndList();
}

void myPreSyncFun()
{
	if( gEngine->isMaster() )
	{
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

		/*
		if( jumpingButton ) {
			pos += (jumpingHeight3 + view);
			printf("JUMP!");
		}
		*/

		/*
			To get a first person camera, the world needs
			to be transformed around the users head.

			This is done by:
			1, Transform the user to coordinate system origin
			2, Apply navigation
			3, Apply rotation
			4, Transform the user back to original position

			However, mathwise this process need to be reversed
			due to the matrix multiplication order.
		*/

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

void myDrawFun()
{
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	
	glMultMatrixf(glm::value_ptr(xform.getVal()));
	glCallList(myLandscapeDisplayList);

	

	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);
}

void myEncodeFun()
{
	sgct::SharedData::instance()->writeObj( &xform );
}

void myDecodeFun()
{
	sgct::SharedData::instance()->readObj( &xform );
}

void keyCallback(int key, int action)
{
	if( gEngine->isMaster() )
	{
		switch( key )
		{
		case SGCT_KEY_UP:
		case SGCT_KEY_W:
			arrowButtons[FORWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
			break;

		case SGCT_KEY_DOWN:
		case SGCT_KEY_S:
			arrowButtons[BACKWARD] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
			break;

		case SGCT_KEY_LEFT:
		case SGCT_KEY_A:
			arrowButtons[LEFT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
			break;

		case SGCT_KEY_RIGHT:
		case SGCT_KEY_D:
			arrowButtons[RIGHT] = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
			break;

		case SGCT_KEY_SPACE:
			jumpingButton = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
			break;

		case SGCT_KEY_LEFT_SHIFT:
		case SGCT_KEY_RIGHT_SHIFT:
			runningButton = ((action == SGCT_REPEAT || action == SGCT_PRESS) ? true : false);
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

void drawXZGrid(int size, float yPos)
{
	glPolygonOffset(0.0f, 0.0f); //offset to avoid z-buffer fighting
	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTranslatef(0.0f, yPos, 0.0f);

    glLineWidth(1.0f);

	glColor4f(0.0f, 153.0/255.0f, 0.0f, 0.8f);
    
	glBegin( GL_LINES );
	for(int x = -(size/2); x < (size/2); x++)
	{
		glVertex3i(x, 0, -(size/2));
		glVertex3i(x, 0, (size/2));
	}

	for(int z = -(size/2); z < (size/2); z++)
	{
		glVertex3i(-(size/2), 0, z);
		glVertex3i((size/2), 0, z);
	}
	glEnd();

	glDisable(GL_BLEND);
	glPopMatrix();
}

void drawPyramid(float width)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonOffset(1.0f, 0.1f); //offset to avoid z-buffer fighting
	//enhance the pyramids with lines in the edges
	glLineWidth(1.0f);
	glColor4f(1.0f, 0.0f, 0.5f, 0.8f);

	glBegin(GL_LINE_LOOP);
	glVertex3f(-width/2.0f, 0.0f, -width/2.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(-width/2.0f, 0.0f, width/2.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-width/2.0f, 0.0f, width/2.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(width/2.0f, 0.0f, width/2.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(width/2.0f, 0.0f, width/2.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(width/2.0f, 0.0f, -width/2.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(width/2.0f, 0.0f, -width/2.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(-width/2.0f, 0.0f, -width/2.0f);
	glEnd();

	glColor4f(1.0f, 0.0f, 0.5f, 0.3f);

	glPolygonOffset(0.0f, 0.0f); //offset to avoid z-buffer fighting
	glBegin(GL_TRIANGLE_FAN);
	//draw top
	glVertex3f(0.0f, 2.0f, 0.0f);

	//draw sides
	glVertex3f(-width / 2.0f, 0.0f, -width / 2.0f);
	glVertex3f(-width / 2.0f, 0.0f, width / 2.0f);
	glVertex3f(width / 2.0f, 0.0f, width / 2.0f);
	glVertex3f(width / 2.0f, 0.0f, -width / 2.0f);
	glVertex3f(-width / 2.0f, 0.0f, -width / 2.0f);

	glEnd();

	glDisable(GL_BLEND);
}

//Used for loading .obj files
bool loadOBJ(const char * path, std::vector < glm::vec3 > & out_vertices, std::vector < glm::vec2 > & out_uvs, std::vector < glm::vec3 > & out_normals)
{
    //Temporary vectors
    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    std::vector< glm::vec3 > temp_vertices;
    std::vector< glm::vec2 > temp_uvs;
    std::vector< glm::vec3 > temp_normals;
    
    //Test if file could be opened
    FILE * file = fopen(path, "r");
    if( file == NULL ){
        printf("Impossible to open the file !\n");
        return false;
    }
    
    while( 1 ){
        
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.
        
        //Vertex
        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }
        
        //Texture Coords
        else if ( strcmp( lineHeader, "vt" ) == 0 ){
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            temp_uvs.push_back(uv);
        }
        
        //Normals of vertex
        else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }
        
        //Face
        else if ( strcmp( lineHeader, "f" ) == 0 ){
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
    
    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++){
        unsigned int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
        out_vertices.push_back(vertex);
    }
    return true;
}






