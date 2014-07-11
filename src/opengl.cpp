#include "opengl.hpp"
#include <iostream>

const char * GetGLErrorMessage(GLenum error_no) {
    switch(error_no) {
    case GL_NO_ERROR:
        return "No error";
    case GL_INVALID_ENUM:
        return "Invalid Enum";
    case GL_INVALID_VALUE:
        return "Invalid value";
    case GL_INVALID_OPERATION:
        return "Invalid operation";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "Invalid Framebuffer operation";
    case GL_OUT_OF_MEMORY:
        return "Out of memory";
    }
    return "Unknown error";
}

GLenum ShowGLError(const char * file, int line) {
    GLenum error_no = glGetError();
    if(error_no != GL_NO_ERROR) {
        // TODO use logger
        std::cerr << "[GL-ERROR] " << file << ':' << line << ": " << GetGLErrorMessage(error_no) << '\n';
    }
    return error_no;
}
