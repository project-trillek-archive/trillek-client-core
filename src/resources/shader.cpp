//A simple class for handling GLSL shader compilation
//Author: Movania Muhammad Mobeen
//Last Modified: February 2, 2011

#include "resources/shader.hpp"
#include <iostream>
#include <fstream>

namespace trillek {
namespace graphics {

Shader::Shader(void) {
    _totalShaders = 0;
    _shaders[VERTEX_SHADER_INDEX] = 0;
    _shaders[FRAGMENT_SHADER_INDEX] = 0;
    _shaders[GEOMETRY_SHADER_INDEX] = 0;
    _attributeList.clear();
    _uniformLocationList.clear();
    _program = 0;
}

Shader::~Shader(void) {
    _attributeList.clear();
    _uniformLocationList.clear();
}

bool Shader::Initialize(const std::vector<Property> &properties) {
    std::string vertex_filename = "";
    std::string fragment_filename = "";

    for (auto propitr = properties.begin(); propitr != properties.end(); ++propitr) {
        const Property*  p = &(*propitr);
        if (p->GetName() == "vertex") {
            vertex_filename = p->Get<std::string>();
        }
        else if (p->GetName() == "fragment") {
            fragment_filename = p->Get<std::string>();
        }
    }

    LoadFromFile(GL_VERTEX_SHADER, vertex_filename);
    LoadFromFile(GL_FRAGMENT_SHADER, fragment_filename);
    CreateAndLinkProgram();

    return isLoaded();
}

void Shader::LoadFromString(GLenum type, const std::string source) {
    GLuint shader = glCreateShader(type);

    const char * ptmp = source.c_str();
    glShaderSource(shader, 1, &ptmp, NULL);

    //check whether the shader loads fine
    GLint status;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *infoLog = new GLchar[infoLogLength];
        glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
        std::cerr << "Shader Compile: " << infoLog << '\n';
        delete[] infoLog;
    }
    _shaders[_totalShaders++] = shader;
}


void Shader::CreateAndLinkProgram() {
    _program = glCreateProgram();
    if (_shaders[VERTEX_SHADER_INDEX] != 0) {
        glAttachShader(_program, _shaders[VERTEX_SHADER_INDEX]);
    }
    if (_shaders[FRAGMENT_SHADER_INDEX] != 0) {
        glAttachShader(_program, _shaders[FRAGMENT_SHADER_INDEX]);
    }
    if (_shaders[GEOMETRY_SHADER_INDEX] != 0) {
        glAttachShader(_program, _shaders[GEOMETRY_SHADER_INDEX]);
    }

    //link and check whether the program links fine
    GLint status;

    // Setup output for multiple render targets
    glBindFragDataLocation(_program, 0, "out_Color");
    glBindFragDataLocation(_program, 1, "out_Normal");
    glBindFragDataLocation(_program, 2, "out_Depth");

    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;

        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *infoLog = new GLchar[infoLogLength];
        glGetProgramInfoLog(_program, infoLogLength, NULL, infoLog);
        std::cerr << "Shader Link: " << infoLog << '\n';
        delete[] infoLog;
    }

    glDeleteShader(_shaders[VERTEX_SHADER_INDEX]);
    glDeleteShader(_shaders[FRAGMENT_SHADER_INDEX]);
    glDeleteShader(_shaders[GEOMETRY_SHADER_INDEX]);
}

void Shader::Use() {
    glUseProgram(_program);
}

void Shader::UnUse() {
    glUseProgram(0);
}

void Shader::AddAttribute(const std::string attribute) {
    _attributeList[attribute] = glGetAttribLocation(_program, attribute.c_str());
}

//An indexer that returns the location of the attribute
GLuint Shader::operator [](const std::string attribute) {
    return _attributeList[attribute];
}

void Shader::AddUniform(const std::string uniform) {
    _uniformLocationList[uniform] = glGetUniformLocation(_program, uniform.c_str());
}

GLuint Shader::operator()(const std::string uniform) {
    return _uniformLocationList[uniform];
}
GLuint Shader::GetProgram() const {
    return _program;
}

void Shader::LoadFromFile(GLenum whichShader, const std::string filename) {
    std::ifstream fp;
    fp.open(filename.c_str(), std::ios_base::in);
    if (fp) {
        std::string buffer(std::istreambuf_iterator<char>(fp), (std::istreambuf_iterator<char>()));
        //copy to source
        LoadFromString(whichShader, buffer);
    }
}

} // End of resource
} // End of trillek
