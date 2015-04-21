/*
 * Framework library functions for the course TNM046
 * Author: Stefan Gustavson 2013-2014 (stefan.gustavson@liu.se)
 * This code is in the public domain.
 */

#ifdef __WIN32__

#include <GL/glext.h>
/* Global function pointers for everything we need beyond OpenGL 1.1.
 * This is a requirement from Microsoft Windows. Other platforms have
 * proper built-in support for modern OpenGL, and don't need this.
 */
extern PFNGLCREATEPROGRAMPROC            glCreateProgram;
extern PFNGLDELETEPROGRAMPROC            glDeleteProgram;
extern PFNGLUSEPROGRAMPROC               glUseProgram;
extern PFNGLCREATESHADERPROC             glCreateShader;
extern PFNGLDELETESHADERPROC             glDeleteShader;
extern PFNGLSHADERSOURCEPROC             glShaderSource;
extern PFNGLCOMPILESHADERPROC            glCompileShader;
extern PFNGLGETSHADERIVPROC              glGetShaderiv;
extern PFNGLGETPROGRAMIVPROC             glGetProgramiv;
extern PFNGLATTACHSHADERPROC             glAttachShader;
extern PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog;
extern PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog;
extern PFNGLLINKPROGRAMPROC              glLinkProgram;
extern PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation;
extern PFNGLUNIFORM1FPROC                glUniform1f;
extern PFNGLUNIFORM1FVPROC               glUniform1fv;
extern PFNGLUNIFORM1IPROC                glUniform1i;
extern PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv;
extern PFNGLGENBUFFERSPROC               glGenBuffers;
extern PFNGLISBUFFERPROC                 glIsBuffer;
extern PFNGLBINDBUFFERPROC               glBindBuffer;
extern PFNGLBUFFERDATAPROC               glBufferData;
extern PFNGLDELETEBUFFERSPROC            glDeleteBuffers;
extern PFNGLGENVERTEXARRAYSPROC          glGenVertexArrays;
extern PFNGLISVERTEXARRAYPROC            glIsVertexArray;
extern PFNGLBINDVERTEXARRAYPROC          glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC       glDeleteVertexArrays;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLGENERATEMIPMAPPROC           glGenerateMipmap;

#endif

namespace tnm061 {
/*
 * printError() - Signal an error.
 * Simple printf() to console for portability.
 */
void printError(const char *errtype, const char *errmsg);

/*
 * loadExtensions() - Load OpenGL extensions for anything above OpenGL
 * version 1.1. (This is a requirement only on Windows, so on other
 * platforms, this function does nothing.)
 */
void loadExtensions();

/*
 * displayFPS() - Calculate, display and return frame rate statistics.
 * Called every frame, but statistics are updated only once per second.
 * The time per frame is a better measure of performance than the
 * number of frames per second, so both are displayed.
 *
 * NOTE: This function doesn't work properly if you call it for multiple
 * windows. Call it only for one window, and only once for each frame.
 */
double displayFPS(GLFWwindow *window);

}
