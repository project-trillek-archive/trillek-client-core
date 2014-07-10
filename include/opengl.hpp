#ifndef OPENGL_HPP_INCLUDED
#define OPENGL_HPP_INCLUDED

#ifndef __APPLE__
#include <GL/glew.h>
#else
#include <OpenGL/gl3.h>
#endif

const char * GetGLErrorMessage(GLenum error_no);
GLenum ShowGLError(const char *, int);
#define CheckGLError() ShowGLError(__FILE__, __LINE__)

#endif
