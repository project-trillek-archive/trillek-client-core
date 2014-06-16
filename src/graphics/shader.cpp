#include "graphics/shader.hpp"
#include <iostream>
#include <fstream>

namespace trillek {
namespace graphics {

Shader::Shader() {
    program = 0;
}

Shader::~Shader() {
    DeleteProgram();
}

void Shader::DeleteProgram() {
    if(program != 0) {
        glDeleteProgram(program);
    }
    for(auto shaderid_itr : shaders) {
        glDeleteShader(shaderid_itr);
    }
    shaders.clear();
    program = 0;
}

void Shader::SetOutputBinding(ShaderOutputType outtype) {
    if(program == 0) {
        program = glCreateProgram();
    }
    switch(outtype) {
    case ShaderOutputType::DEFAULT_3TARGET:
    default:
        // Setup output for multiple render targets
        glBindFragDataLocation(program, 0, "out_Color");
        glBindFragDataLocation(program, 1, "out_Normal");
        glBindFragDataLocation(program, 2, "out_Depth");
        break;
    }
}

bool Shader::LinkProgram() {
    if(program == 0) {
        program = glCreateProgram();
        SetOutputBinding(ShaderOutputType::DEFAULT_3TARGET);
    }

    // attach all shaders
    for(auto shaderid_itr : shaders) {
        glAttachShader(program, shaderid_itr);
    }

    //link and check if the program links ok
    bool linkok = true;
    GLint status;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *infoLog = new GLchar[infoLogLength];
        glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
        std::cerr << "Shader Link: " << infoLog << '\n';
        delete[] infoLog;
        linkok = false;
    }

    for(auto shaderid_itr : shaders) {
        glDeleteShader(shaderid_itr);
    }
    shaders.clear();
    return linkok;
}

void Shader::Use() {
    glUseProgram(program);
}

void Shader::UnUse() {
    glUseProgram(0);
}

//An indexer that returns the location of the attribute
GLuint Shader::operator [](const std::string & attribute) {
    auto attrib = attributes_list.find(attribute);
    if(attrib == attributes_list.end()) {
        GLuint attrib_id = glGetAttribLocation(program, attribute.c_str());
        if(attrib_id) {
            attributes_list[attribute] = attrib_id;
        }
        return attrib_id;
    }
    return attrib->second;
}

GLuint Shader::operator()(const std::string &uniform) {
    auto uniform_itr = uniforms_list.find(uniform);
    if(uniform_itr == uniforms_list.end()) {
        GLuint uniform_id = glGetUniformLocation(program, uniform.c_str());
        if(uniform_id) {
            uniforms_list[uniform] = uniform_id;
        }
        return uniform_id;
    }
    return uniform_itr->second;
}
GLuint Shader::GetProgram() {
    return program;
}

void Shader::LoadFromString(ShaderType type, const std::string &source) {
    GLuint shader = glCreateShader(type);

    GLint slen = source.length();
    const GLchar *sstr = source.data();
    glShaderSource(shader, 1, &sstr, &slen);

    // check if the shader compiles
    GLint status;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        GLchar *info_log = new GLchar[log_length];
        glGetShaderInfoLog(shader, log_length, NULL, info_log);
        std::cerr << "Shader Compile: " << info_log << '\n';
        delete[] info_log;
    }
    shaders.push_back(shader);
}

void Shader::LoadFromFile(ShaderType whichShader, const std::string & filename) {
    std::ifstream fp;
    fp.open(filename, std::ios_base::in);
    if(fp.is_open()) {
        std::string buffer(std::istreambuf_iterator<char>(fp), (std::istreambuf_iterator<char>()));
        // compile source
        LoadFromString(whichShader, buffer);
    }
}

} // End of graphics
} // End of trillek
