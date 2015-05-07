#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

#ifndef __APPLE__
PFNGLACTIVETEXTUREARBPROC              glActiveTextureARB;

PFNGLGETOBJECTPARAMETERIVARBPROC       glGetObjectParameterivARB;
PFNGLBINDATTRIBLOCATIONARBPROC         glBindAttribLocationARB;
PFNGLUSEPROGRAMOBJECTARBPROC           glUseProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC         glCreateShaderObjectARB;
PFNGLCREATEPROGRAMOBJECTARBPROC        glCreateProgramObjectARB;
PFNGLVALIDATEPROGRAMARBPROC            glValidateProgramARB;
PFNGLSHADERSOURCEARBPROC               glShaderSourceARB;
PFNGLCOMPILESHADERARBPROC              glCompileShaderARB;
PFNGLATTACHOBJECTARBPROC               glAttachObjectARB;
PFNGLLINKPROGRAMARBPROC                glLinkProgramARB;
PFNGLGETINFOLOGARBPROC                 glGetInfoLogARB;
PFNGLDELETEOBJECTARBPROC               glDeleteObjectARB;

PFNGLGETUNIFORMLOCATIONARBPROC         glGetUniformLocationARB;
PFNGLUNIFORM1IARBPROC                  glUniform1iARB;
PFNGLUNIFORM1FARBPROC                  glUniform1fARB;
PFNGLUNIFORM2FARBPROC                  glUniform2fARB;
PFNGLUNIFORM3FARBPROC                  glUniform3fARB;
PFNGLUNIFORM4FARBPROC                  glUniform4fARB;

PFNGLGENFRAMEBUFFERSEXTPROC            glGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC            glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC         glDeleteFramebuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC       glFramebufferTexture2DEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC     glCheckFramebufferStatusEXT;

PFNGLGENBUFFERSARBPROC                 glGenBuffersARB;
PFNGLBINDBUFFERARBPROC                 glBindBufferARB;
PFNGLMAPBUFFERARBPROC                  glMapBufferARB;
PFNGLUNMAPBUFFERARBPROC                glUnmapBufferARB;
PFNGLBUFFERDATAARBPROC                 glBufferDataARB;
PFNGLDELETEBUFFERSARBPROC              glDeleteBuffersARB;
#endif

//-----------------------------------------------------------------------------

#define init_proc(t, n) if (!(n = (t) glGetProcAddress(#n))) \
                            std::runtime_error(#n)

void init_ext(const char *needle)
{
    const GLubyte *haystack, *c;

    // Search for the given string in the OpenGL extension strings.

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return;
    }

    throw std::runtime_error(needle);
}

void init_ogl()
{
    init_ext("ARB_vertex_shader");
    init_ext("ARB_shader_objects");
    init_ext("ARB_fragment_shader");
    init_ext("ARB_vertex_buffer_object");
    init_ext("ARB_pixel_buffer_object");
    init_ext("EXT_framebuffer_object");

#ifndef __APPLE__
    init_proc(PFNGLACTIVETEXTUREARBPROC,          glActiveTextureARB);

    init_proc(PFNGLGETOBJECTPARAMETERIVARBPROC,   glGetObjectParameterivARB);
    init_proc(PFNGLBINDATTRIBLOCATIONARBPROC,     glBindAttribLocationARB);
    init_proc(PFNGLUSEPROGRAMOBJECTARBPROC,       glUseProgramObjectARB);
    init_proc(PFNGLCREATESHADEROBJECTARBPROC,     glCreateShaderObjectARB);
    init_proc(PFNGLCREATEPROGRAMOBJECTARBPROC,    glCreateProgramObjectARB);
    init_proc(PFNGLVALIDATEPROGRAMARBPROC,        glValidateProgramARB);
    init_proc(PFNGLSHADERSOURCEARBPROC,           glShaderSourceARB);
    init_proc(PFNGLCOMPILESHADERARBPROC,          glCompileShaderARB);
    init_proc(PFNGLATTACHOBJECTARBPROC,           glAttachObjectARB);
    init_proc(PFNGLLINKPROGRAMARBPROC,            glLinkProgramARB);
    init_proc(PFNGLGETINFOLOGARBPROC,             glGetInfoLogARB);
    init_proc(PFNGLDELETEOBJECTARBPROC,           glDeleteObjectARB);

    init_proc(PFNGLGETUNIFORMLOCATIONARBPROC,     glGetUniformLocationARB);
    init_proc(PFNGLUNIFORM1IARBPROC,              glUniform1iARB);
    init_proc(PFNGLUNIFORM1FARBPROC,              glUniform1fARB);
    init_proc(PFNGLUNIFORM2FARBPROC,              glUniform2fARB);
    init_proc(PFNGLUNIFORM3FARBPROC,              glUniform3fARB);
    init_proc(PFNGLUNIFORM4FARBPROC,              glUniform4fARB);

    init_proc(PFNGLGENFRAMEBUFFERSEXTPROC,        glGenFramebuffersEXT);
    init_proc(PFNGLBINDFRAMEBUFFEREXTPROC,        glBindFramebufferEXT);
    init_proc(PFNGLDELETEFRAMEBUFFERSEXTPROC,     glDeleteFramebuffersEXT);
    init_proc(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC,   glFramebufferTexture2DEXT);
    init_proc(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
    init_proc(PFNGLGENBUFFERSARBPROC,             glGenBuffersARB);
    init_proc(PFNGLBINDBUFFERARBPROC,             glBindBufferARB);
    init_proc(PFNGLMAPBUFFERARBPROC,              glMapBufferARB);
    init_proc(PFNGLUNMAPBUFFERARBPROC,            glUnmapBufferARB);
    init_proc(PFNGLBUFFERDATAARBPROC,             glBufferDataARB);
    init_proc(PFNGLDELETEBUFFERSARBPROC,          glDeleteBuffersARB);
#endif
}

//-----------------------------------------------------------------------------

void check_ogl(const char *file, int line)
{
    GLenum err = glGetError();

    if (err != GL_NO_ERROR)
        std::cerr << file << ":"
                  << line << " "
                  << (const char *) gluErrorString(err) << std::endl;
}

//-----------------------------------------------------------------------------

fbo::fbo(GLint color_format,
         GLint depth_format, GLsizei w, GLsizei h) : _w(w), _h(h)
{
    GLint o[1], v[4];

    get_framebuffer(o, v);

    glGenFramebuffersEXT(1, &frame);
    glGenTextures       (1, &color);
    glGenTextures       (1, &depth);

    // Initialize the color render buffer object.

    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D (GL_TEXTURE_2D, 0, color_format, w, h, 0,
                  GL_RGBA, GL_INT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    // Initialize the depth render buffer object.

    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D (GL_TEXTURE_2D, 0, depth_format, w, h, 0,
                  GL_DEPTH_COMPONENT, GL_INT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB,
                                   GL_COMPARE_R_TO_TEXTURE_ARB);

    // Initialize the frame buffer object.

    glBindFramebufferEXT     (GL_FRAMEBUFFER_EXT, frame);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D, color, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_DEPTH_ATTACHMENT_EXT,
                              GL_TEXTURE_2D, depth, 0);

    // Confirm the frame buffer object status.

    switch (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT))
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        break; 
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer incomplete attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer missing attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
        throw std::runtime_error("Framebuffer duplicate attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        throw std::runtime_error("Framebuffer dimensions");
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        throw std::runtime_error("Framebuffer formats");
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        throw std::runtime_error("Framebuffer draw buffer");
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        throw std::runtime_error("Framebuffer read buffer");
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        throw std::runtime_error("Framebuffer unsupported");
    default:
        throw std::runtime_error("Framebuffer error");
    }

    // Zero the buffer.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    set_framebuffer(o, v);

    GL_CHECK();
}

fbo::~fbo()
{
    glDeleteFramebuffersEXT(1, &frame);
    glDeleteTextures       (1, &depth);
    glDeleteTextures       (1, &color);

    GL_CHECK();
}

void fbo::bind_frame()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame);
    glViewport(1, 1, _w - 2, _h - 2);
    glScissor (1, 1, _w - 2, _h - 2);

    GL_CHECK();
}

void fbo::bind_color(GLenum T)
{
    glActiveTextureARB(T);
    {
        glBindTexture(GL_TEXTURE_2D, color);
    }
    glActiveTextureARB(GL_TEXTURE0);

    GL_CHECK();
}

void fbo::bind_depth(GLenum T)
{
    glActiveTextureARB(T);
    {
        glBindTexture(GL_TEXTURE_2D, depth);
    }
    glActiveTextureARB(GL_TEXTURE0);

    GL_CHECK();
}

//-----------------------------------------------------------------------------

vbo::vbo(GLenum target, GLenum usage, GLsizei size)
{
    glGenBuffersARB(1, &buffer);

    glBindBufferARB(target, buffer);
    glBufferDataARB(target, size, NULL, usage);

    glBindBufferARB(target, 0);

    GL_CHECK();
}

vbo::~vbo()
{
    glDeleteBuffersARB(1, &buffer);
    GL_CHECK();
}

void vbo::bind(GLenum target)
{
    glBindBufferARB(target, buffer);
    GL_CHECK();
}

//-----------------------------------------------------------------------------

void shader::check_log(GLhandleARB handle)
{
    char *log;
    GLint len;

    // Dump the contents of the log, if any.

    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &len);

    if ((len > 1) && (log = new char[len + 1]))
    {
        glGetInfoLogARB(handle, len, NULL, log);

        std::cerr << log << std::endl;

        delete [] log;
    }
}

shader::shader(std::string vert_str, std::string frag_str)
{
    prog = glCreateProgramObjectARB();
    vert = 0;
    frag = 0;

    // Compile the vertex shader.

    if (vert_str.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) vert_str.c_str();

        vert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        glShaderSourceARB (vert, 1, &p, 0);
        glCompileShaderARB(vert);
        
        check_log(vert);
    }

    // Compile the frag shader.

    if (frag_str.length() > 0)
    {
        const GLcharARB *p = (const GLcharARB *) frag_str.c_str();

        frag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        glShaderSourceARB (frag, 1, &p, 0);
        glCompileShaderARB(frag);
        
        check_log(frag);
    }

    // Link these shader objects to a program object.

    glAttachObjectARB(prog, vert);
    glAttachObjectARB(prog, frag);

    glLinkProgramARB(prog);   check_log(prog);

    GL_CHECK();
}

shader::~shader()
{
    glDeleteObjectARB(prog);
    glDeleteObjectARB(vert);
    glDeleteObjectARB(frag);
    GL_CHECK();
}

void shader::bind()
{
    glUseProgramObjectARB(prog);
    GL_CHECK();
}

//-----------------------------------------------------------------------------

void shader::uniform(std::string name, int d)
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1iARB(loc, d);
}

void shader::uniform(std::string name, float a)
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform1fARB(loc, a);
}

void shader::uniform(std::string name, float a, float b)
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform2fARB(loc, a, b);
}

void shader::uniform(std::string name, float a, float b, float c)
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform3fARB(loc, a, b, c);
}

void shader::uniform(std::string name, float a, float b, float c, float d)
{
    int loc;

    if ((loc = glGetUniformLocationARB(prog, name.c_str())) >= 0)
        glUniform4fARB(loc, a, b, c, d);
}

//-----------------------------------------------------------------------------

void get_framebuffer(GLint o[1], GLint v[4])
{
    glGetIntegerv(GL_VIEWPORT,                v);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, o);
}

void set_framebuffer(GLint o[1], GLint v[4])
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, o[0]);
    glViewport(v[0], v[1], v[2], v[3]);
}

//-----------------------------------------------------------------------------
