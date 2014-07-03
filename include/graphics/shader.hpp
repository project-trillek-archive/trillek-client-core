#ifndef SHADER_HPP_INCLUDED
#define SHADER_HPP_INCLUDED

#include "opengl.hpp"
#include "type-id.hpp"
#include <map>
#include <string>
#include <vector>
#include <atomic>
#include "systems/resource-system.hpp"
#include "graphics-base.hpp"

namespace trillek {
namespace graphics {

// fix for building on slightly older glew
#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER 0x91B9
#endif

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

    virtual bool SystemStart(const std::list<Property> &);
    virtual bool SystemReset(const std::list<Property> &);

    /**
     * \brief parse a shader from json
     * \param[in] const std::string& shader_name the name of the new shader
     * \param[in] rapidjson::Value& node The node to parse.
     * \return false on errors, true for success
     */
    virtual bool Parse(const std::string &shader_name, const rapidjson::Value& node);

    /**
     * \brief Serialize this shader to the provided JSON node.
     *
     * \param[in] rapidjson::Document& document The document to serialize to.
     * \return bool False if an error occured in serializing.
     */
    virtual bool Serialize(rapidjson::Document& document);

    bool ParseDefines(std::string &defstring, const rapidjson::Value& node);
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
    static void UnUse();
    GLuint GetProgram();

    // ISSUE: This is a bit questionable as it violates the principle of least surprise
    //An indexer that returns the location of the attribute/uniform
    GLint operator[](const std::string & attribute);
    GLint operator()(const std::string & uniform);

    GLint Attribute(const std::string & attribute);
    GLint Uniform(const std::string & uniform);

    //Program deletion
    void DeleteProgram();
    bool isLoaded() { return program != 0; }

    static void InitializeTypes();
private:
    GLuint program;
    std::vector<GLuint> shaders;
    std::vector<std::pair<std::string, GLuint>> output_bindings;
    std::map<std::string, GLint> attributes_list;
    std::map<std::string, GLint> uniforms_list;

    static std::once_flag types_once;
    static std::map<std::string, ShaderType> shaderclass;
};

} // End of graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NAME(graphics::Shader, "shaders", 401)
} // namespace reflection

} // End of trillek

#endif
