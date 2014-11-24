#include "graphics/shader.hpp"
#include "resources/text-file.hpp"
#include <iostream>
#include <fstream>
#include "logging.hpp"

namespace trillek {
namespace graphics {

Shader::Shader() {
    program = 0;
}

Shader::~Shader() {
    DeleteProgram();
}

std::once_flag Shader::types_once;
std::map<std::string, ShaderType> Shader::shaderclass;

void Shader::InitializeTypes() {
    std::call_once(Shader::types_once,
        [ ] () {
        Shader::shaderclass["vertex"] = VERTEX_SHADER;
        Shader::shaderclass["fragment"] = FRAGMENT_SHADER;
        Shader::shaderclass["geometry"] = GEOMETRY_SHADER;
        Shader::shaderclass["tess-cntl"] = TESS_CONTROL_SHADER;
        Shader::shaderclass["tess-eval"] = TESS_EVAL_SHADER;
        Shader::shaderclass["compute"] = COMPUTE_SHADER;

    });
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

bool Shader::SystemStart(const std::list<Property> &) {
    return false;
}
bool Shader::SystemReset(const std::list<Property> &) {
    return false;
}
bool Shader::Serialize(rapidjson::Document& document) {
    return false;
}

void Shader::SetOutputBinding(ShaderOutputType outtype) {

    if(program == 0) {
        program = glCreateProgram();
        CheckGLError();
    }
    if(output_bindings.size() > 0 && outtype == ShaderOutputType::DEFAULT_TARGETS) {
        for(auto bindpair : output_bindings) {
            glBindFragDataLocation(program, bindpair.second, bindpair.first.c_str());
            CheckGLError();
        }
        return;
    }
    switch(outtype) {
    case ShaderOutputType::DEFAULT_TARGETS:
    default:
        // Setup output for multiple render targets
        glBindFragDataLocation(program, 0, "out_col"); CheckGLError();
        glBindFragDataLocation(program, 1, "out_norm"); CheckGLError();
        glBindFragDataLocation(program, 2, "out_Depth"); CheckGLError();
        break;
    }
}

bool Shader::LinkProgram() {
    if(program == 0) {
        program = glCreateProgram(); CheckGLError();
        SetOutputBinding(ShaderOutputType::DEFAULT_TARGETS);
    }

    // attach all shaders
    for(auto shaderid_itr : shaders) {
        glAttachShader(program, shaderid_itr);
        CheckGLError();
    }

    //link and check if the program links ok
    bool linkok = true;
    GLint status;
    glLinkProgram(program);
    CheckGLError();
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *infoLog = new GLchar[infoLogLength];
        glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
        LOGMSGC(ERROR) << "Shader Link: " << infoLog;
        delete[] infoLog;
        linkok = false;
    }
    CheckGLError();

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

GLint Shader::Attribute(const std::string & attribute) {
    auto attrib = attributes_list.find(attribute);
    if(attrib == attributes_list.end()) {
        GLint attrib_id = glGetAttribLocation(program, attribute.c_str());
        if(attrib_id) {
            attributes_list[attribute] = attrib_id;
        }
        return attrib_id;
    }
    return attrib->second;
}

GLint Shader::Uniform(const std::string & uniform) {
    auto uniform_itr = uniforms_list.find(uniform);
    if(uniform_itr == uniforms_list.end()) {
        GLint uniform_id = glGetUniformLocation(program, uniform.c_str());
        if(uniform_id) {
            uniforms_list[uniform] = uniform_id;
        }
        return uniform_id;
    }
    return uniform_itr->second;
}

//An indexer that returns the location of the attribute
GLint Shader::operator [](const std::string & attribute) {
    auto attrib = attributes_list.find(attribute);
    if(attrib == attributes_list.end()) {
        GLint attrib_id = glGetAttribLocation(program, attribute.c_str());
        if(attrib_id) {
            attributes_list[attribute] = attrib_id;
        }
        return attrib_id;
    }
    return attrib->second;
}

GLint Shader::operator()(const std::string &uniform) {
    auto uniform_itr = uniforms_list.find(uniform);
    if(uniform_itr == uniforms_list.end()) {
        GLint uniform_id = glGetUniformLocation(program, uniform.c_str());
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

bool Shader::ParseDefines(std::string &defstring, const rapidjson::Value& node) {
    for(auto sdef_itr = node.MemberBegin();
            sdef_itr != node.MemberEnd(); sdef_itr++) {
        std::string define_name(sdef_itr->name.GetString(), sdef_itr->name.GetStringLength());
        if(sdef_itr->value.IsNumber()) {
            std::string define_val(sdef_itr->name.GetString(), sdef_itr->name.GetStringLength());
            // add a valued define
            defstring.append("#define ").append(define_name);
            defstring.append(" ").append(define_val).append("\n");
        }
        else if(sdef_itr->value.IsNull()) {
            // add a blank define
            defstring.append("#define ").append(define_name).append("\n");
        }
        else {
            // invalid
            LOGMSGC(ERROR) << "Invalid shader define";
            return false;
        }
    }
    return true;
}

bool Shader::Parse(const std::string &shader_name, const rapidjson::Value& node) {
    std::string globdefines;

    if(!node.IsObject()) {
        LOGMSGC(WARNING) << "Invalid shader entry";
        return false;
    }
    for(auto shade_param_itr = node.MemberBegin();
            shade_param_itr != node.MemberEnd(); shade_param_itr++) {
        std::string param_name(shade_param_itr->name.GetString(), shade_param_itr->name.GetStringLength());
        if(shade_param_itr->value.IsString()) {
            std::string param_val(shade_param_itr->value.GetString(), shade_param_itr->value.GetStringLength());

            auto param_scan = Shader::shaderclass.find(param_name);
            if(param_scan != Shader::shaderclass.end()) {
                auto textdata = resource::ResourceMap::Get<resource::TextFile>(param_val);
                if(textdata) {
                    std::string shadertext = textdata->GetText();
                    std::vector<std::string> shadersrc;
                    shadersrc.push_back(Shader::VersionPrePass(shadertext));
                    shadersrc.push_back(globdefines);
                    shadersrc.push_back(shadertext);
                    LoadFromStrings(param_scan->second, shadersrc);
                }
            }
        }
        else if(shade_param_itr->value.IsObject()) {
            auto param_scan = Shader::shaderclass.find(param_name);
            if(param_scan != Shader::shaderclass.end()) {
                // scan sections
                std::string sectiondefines;
                std::string shadertext;
                for(auto sdef_itr = shade_param_itr->value.MemberBegin();
                        sdef_itr != shade_param_itr->value.MemberEnd(); sdef_itr++) {
                    std::string ssec_name(sdef_itr->name.GetString(), sdef_itr->name.GetStringLength());
                    if(ssec_name == "define") {
                        if(sdef_itr->value.IsObject()) {
                            if(!ParseDefines(sectiondefines, sdef_itr->value)) {
                                return false;
                            }
                        }
                        else {
                            LOGMSGC(ERROR) << "Invalid define section";
                        }
                    }
                    else if(ssec_name == "src" || ssec_name == "source") {
                        if(sdef_itr->value.IsString()) {
                            std::string ssec_val(sdef_itr->value.GetString(), sdef_itr->value.GetStringLength());
                            auto textdata = resource::ResourceMap::Get<resource::TextFile>(ssec_val);
                            if(textdata) {
                                shadertext = textdata->GetText();
                            }
                            else {
                                LOGMSGC(WARNING) << "Shader source not loaded";
                            }
                        }
                        else {
                            LOGMSGC(ERROR) << "Invalid source section";
                        }
                    }
                    else if(ssec_name == "src-file" || ssec_name == "filename") {
                        if(sdef_itr->value.IsString()) {
                            std::string ssec_val(sdef_itr->value.GetString(), sdef_itr->value.GetStringLength());
                            std::vector<Property> fileprop;
                            fileprop.push_back(Property("filename", ssec_val));
                            resource::TextFile textdata;
                            if(textdata.Initialize(fileprop)) {
                                shadertext = textdata.GetText();
                            }
                            else {
                                LOGMSGC(WARNING) << "Shader source not loaded";
                            }
                        }
                        else {
                            LOGMSGC(ERROR) << "Invalid source section";
                        }
                    }
                }
                if(shadertext.length() > 0) {
                    std::vector<std::string> shadersrc;
                    shadersrc.push_back(Shader::VersionPrePass(shadertext));
                    shadersrc.push_back(globdefines);
                    shadersrc.push_back(sectiondefines);
                    shadersrc.push_back(shadertext);
                    LoadFromStrings(param_scan->second, shadersrc);
                }
                else {
                    LOGMSGC(WARNING) << "Empty shader definition";
                }
            }
            else if(param_name == "define") {
                if(!ParseDefines(globdefines, shade_param_itr->value)) {
                    return false;
                }
            }
            else if(param_name == "colorbinding") {
                for(auto sdef_itr = shade_param_itr->value.MemberBegin();
                        sdef_itr != shade_param_itr->value.MemberEnd(); sdef_itr++) {
                    std::string bind_name(sdef_itr->name.GetString(), sdef_itr->name.GetStringLength());
                    if(sdef_itr->value.IsUint()) {
                        GLuint bindid = sdef_itr->value.GetUint();
                        output_bindings.push_back(std::pair<std::string, GLuint>(bind_name, bindid));
                    }
                    else {
                        // invalid
                        LOGMSGC(ERROR) << "Invalid output definition";
                        return false;
                    }
                }
            }
            else {
                LOGMSGC(WARNING) << "Unknown shader parameter: " << param_name;
            }
        }
    }
    return LinkProgram();
}

std::string Shader::VersionPrePass(std::string & source) {
    size_t vpos = source.find("#version", 0);
    if(vpos == std::string::npos) {
        return std::string(); // #version directive not present in source
    }
    size_t epos = source.find('\n', vpos);
    if(epos == std::string::npos) { // #version directive is only thing in source
        std::string line(source);
        source.clear();
        line.append("\n");
        return line;
    }
    std::string line = source.substr(vpos, epos - vpos);
    line.append("\n");
    source.erase(vpos, epos - vpos);
    return line;
}

void Shader::LoadFromStrings(ShaderType type, const std::vector<std::string> &source) {
    GLuint shader = glCreateShader(type);

    if(source.size() < 1) {
        return;
    }
    GLint *slen = new GLint[source.size()];
    const GLchar **sstr = new const GLchar*[source.size()];
    unsigned int i, sz;
    for(i = 0, sz = source.size(); i < sz; i++) {
        slen[i] = source[i].length();
        sstr[i] = source[i].c_str();
    }
    glShaderSource(shader, sz, sstr, slen);
    delete slen;
    delete sstr;

    // check if the shader compiles
    GLint status;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        GLchar *info_log = new GLchar[log_length];
        glGetShaderInfoLog(shader, log_length, NULL, info_log);
        LOGMSGC(ERROR) << "Shader Compile: " << info_log;
        delete[] info_log;
    }
    shaders.push_back(shader);
}

void Shader::LoadFromString(ShaderType type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    CheckGLError();

    GLint slen = source.length();
    const GLchar *sstr = source.data();
    glShaderSource(shader, 1, &sstr, &slen);
    CheckGLError();

    // check if the shader compiles
    GLint status;
    glCompileShader(shader);
    CheckGLError();
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        GLchar *info_log = new GLchar[log_length];
        glGetShaderInfoLog(shader, log_length, NULL, info_log);
        LOGMSGC(ERROR) << "Shader Compile: " << info_log;
        delete[] info_log;
    }
    CheckGLError();
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
