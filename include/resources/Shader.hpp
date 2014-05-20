//A simple class for handling GLSL shader compilation
//Auhtor: Movania Muhammad Mobeen
//Last Modified: February 2, 2011

#ifndef SHADER_HPP_INCLUDED
#define SHADER_HPP_INCLUDED


#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include "GL/glew.h"
#endif

#include <map>
#include <string>
#include "systems/ResourceSystem.h"

namespace trillek {
namespace resource {

class Shader : public ResourceBase {
public:
    Shader(void);
    ~Shader(void);

    virtual bool Initialize(const std::vector<Property> &properties);

    void LoadFromString(GLenum whichShader, const std::string source);
    void LoadFromFile(GLenum whichShader, const std::string filename);
    void CreateAndLinkProgram();
    void Use();
    void UnUse();
    void AddAttribute(const std::string attribute);
    void AddUniform(const std::string uniform);
    GLuint GetProgram() const;

    // ISSUE: This is a bit questionable as it violates the principle of least surprise
    //An indexer that returns the location of the attribute/uniform
    GLuint operator[](const std::string attribute);
    GLuint operator()(const std::string uniform);

    //Program deletion
    void DeleteProgram() { glDeleteProgram(_program); _program = -1; }
    bool isLoaded() { return _program != 0; }
    enum ShaderType { VERTEX_SHADER = GL_VERTEX_SHADER, FRAGMENT_SHADER = GL_FRAGMENT_SHADER, GEOMETRY_SHADER };
private:
    enum ShaderIndex { VERTEX_SHADER_INDEX, FRAGMENT_SHADER_INDEX, GEOMETRY_SHADER_INDEX };
    GLuint	_program;
    int _totalShaders;
    GLuint _shaders[3];//0->vertexshader, 1->fragmentshader, 2->geometryshader
    std::map<std::string, GLuint> _attributeList;
    std::map<std::string, GLuint> _uniformLocationList;
};

} // End of resource

namespace reflection {

    template <> inline const char* GetTypeName<resource::Shader>() { return "Shader"; }
    template <> inline const unsigned int GetTypeID<resource::Shader>() { return 1002; }

} // End of reflection
} // End of trillek

#endif
