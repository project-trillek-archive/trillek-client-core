#ifndef SHADER_HPP_INCLUDED
#define SHADER_HPP_INCLUDED

#include "opengl.hpp"

#include <map>
#include <string>
#include <vector>
#include <atomic>
#include "systems/resource-system.hpp"
#include "graphics-base.hpp"

namespace trillek {
namespace graphics {

enum ShaderType : GLenum {
    VERTEX_SHADER = GL_VERTEX_SHADER,
    FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
    GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
    TESS_CONTROL_SHADER = GL_TESS_CONTROL_SHADER,
    TESS_EVAL_SHADER = GL_TESS_EVALUATION_SHADER,
    COMPUTE_SHADER = GL_COMPUTE_SHADER
};

enum class ShaderOutputType {
    DEFAULT_TARGETS
};

class Shader : public GraphicsBase {
public:
    Shader();
    virtual ~Shader();

    /**
     * \brief parse a shader from json
     * \return false on errors, true for success
     */
    bool Parse(const std::string &shader_name, rapidjson::Value& node);
    bool ParseDefines(std::string &defstring, rapidjson::Value& node);
    void LoadFromString(ShaderType whichShader, const std::string & source);
    void LoadFromStrings(ShaderType whichShader, const std::vector<std::string> & source);
    void LoadFromFile(ShaderType whichShader, const std::string & filename);
    void SetOutputBinding(ShaderOutputType);

    /**
     * \brief Link the program from loaded shader source
     * \return false on link errors, true for success
     */
    bool LinkProgram();
    void Use();
    void UnUse();
    GLuint GetProgram();

    // ISSUE: This is a bit questionable as it violates the principle of least surprise
    //An indexer that returns the location of the attribute/uniform
    GLuint operator[](const std::string & attribute);
    GLuint operator()(const std::string & uniform);

    //Program deletion
    void DeleteProgram();
    bool isLoaded() { return program != 0; }

    static void InitializeTypes();
private:
    GLuint program;
    std::vector<GLuint> shaders;
    std::vector<std::pair<std::string, GLuint>> output_bindings;
    std::map<std::string, GLuint> attributes_list;
    std::map<std::string, GLuint> uniforms_list;

    static std::once_flag types_once;
    static std::map<std::string, ShaderType> shaderclass;
};

} // End of graphics

namespace reflection {

template <> inline const char* GetTypeName<graphics::Shader>() { return "shaders"; }
template <> inline const unsigned int GetTypeID<graphics::Shader>() { return 401; }

} // namespace reflection
} // End of trillek

#endif
