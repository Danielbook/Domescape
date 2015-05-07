#ifndef GL_HPP
#define GL_HPP

#include <string>

//-----------------------------------------------------------------------------

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "glext.h"
#define glGetProcAddress(n) glXGetProcAddressARB((GLubyte *) n)
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"
#define glGetProcAddress(n) wglGetProcAddress(n)
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

//-----------------------------------------------------------------------------

#ifndef __APPLE__
extern PFNGLACTIVETEXTUREARBPROC              glActiveTextureARB;

extern PFNGLGETOBJECTPARAMETERIVARBPROC       glGetObjectParameterivARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC         glBindAttribLocationARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC           glUseProgramObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC         glCreateShaderObjectARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC        glCreateProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC            glValidateProgramARB;
extern PFNGLSHADERSOURCEARBPROC               glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC              glCompileShaderARB;
extern PFNGLATTACHOBJECTARBPROC               glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC                glLinkProgramARB;
extern PFNGLGETINFOLOGARBPROC                 glGetInfoLogARB;
extern PFNGLDELETEOBJECTARBPROC               glDeleteObjectARB;

extern PFNGLGETUNIFORMLOCATIONARBPROC         glGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC                  glUniform1iARB;
extern PFNGLUNIFORM1FARBPROC                  glUniform1fARB;
extern PFNGLUNIFORM2FARBPROC                  glUniform2fARB;
extern PFNGLUNIFORM3FARBPROC                  glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC                  glUniform4fARB;

extern PFNGLGENFRAMEBUFFERSEXTPROC            glGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC            glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC         glDeleteFramebuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC       glFramebufferTexture2DEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC     glCheckFramebufferStatusEXT;

extern PFNGLGENBUFFERSARBPROC                 glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC                 glBindBufferARB;
extern PFNGLMAPBUFFERARBPROC                  glMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC                glUnmapBufferARB;
extern PFNGLBUFFERDATAARBPROC                 glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC              glDeleteBuffersARB;
#endif

void init_ogl();
void check_ogl(const char *, int);

#ifndef NDEBUG
#define GL_CHECK() check_ogl(__FILE__, __LINE__)
#else
#define GL_CHECK() {}
#endif

//-----------------------------------------------------------------------------

class fbo
{
    GLuint frame;
    GLuint color;
    GLuint depth;

    GLsizei _w;
    GLsizei _h;

public:

    fbo(GLint, GLint, GLsizei, GLsizei);
   ~fbo();

    void bind_frame();
    void bind_color(GLenum);
    void bind_depth(GLenum);
};

//-----------------------------------------------------------------------------

class vbo
{
    GLuint buffer;

public:

    vbo(GLenum, GLenum, GLsizei);
   ~vbo();

    void bind(GLenum);
};

//-----------------------------------------------------------------------------

class shader
{
    void check_log(GLhandleARB);

    GLhandleARB vert;
    GLhandleARB frag;
    GLhandleARB prog;

public:

    shader(std::string, std::string);
   ~shader();

    void bind();

    void uniform(std::string, int);
    void uniform(std::string, float);
    void uniform(std::string, float, float);
    void uniform(std::string, float, float, float);
    void uniform(std::string, float, float, float, float);
};

//-----------------------------------------------------------------------------

void get_framebuffer(GLint[1], GLint[4]);
void set_framebuffer(GLint[1], GLint[4]);

//-----------------------------------------------------------------------------

#endif
