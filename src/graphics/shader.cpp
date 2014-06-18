#include "graphics/shader.hpp"
#include "resources/text-file.hpp"
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
    if(output_bindings.size() > 0 && outtype == ShaderOutputType::DEFAULT_TARGETS) {
        for(auto bindpair : output_bindings) {
            glBindFragDataLocation(program, bindpair.second, bindpair.first.c_str());
        }
        return;
    }
    switch(outtype) {
    case ShaderOutputType::DEFAULT_TARGETS:
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
        SetOutputBinding(ShaderOutputType::DEFAULT_TARGETS);
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

bool Shader::Parse(const std::string &shader_name, rapidjson::Value& node) {

    for(auto shade_param_itr = node.MemberBegin();
            shade_param_itr != node.MemberEnd(); shade_param_itr++) {
        std::string param_name(shade_param_itr->name.GetString(), shade_param_itr->name.GetStringLength());
        if(shade_param_itr->value.IsString()) {
            std::string param_val(shade_param_itr->value.GetString(), shade_param_itr->value.GetStringLength());
            if(param_name == "vertex") {
                // get source text
                auto textdata = resource::ResourceMap::Get<resource::TextFile>(param_val);
                if(textdata) {
                    LoadFromString(VERTEX_SHADER, textdata->GetText());
                }
            }
            else if(param_name == "fragment") {
                // get source text
                auto textdata = resource::ResourceMap::Get<resource::TextFile>(param_val);
                if(textdata) {
                    LoadFromString(FRAGMENT_SHADER, textdata->GetText());
                }
            }
            else if(param_name == "geometry") {
                // should check for GL 3.2+
                // get source text
                auto textdata = resource::ResourceMap::Get<resource::TextFile>(param_val);
                if(textdata) {
                    LoadFromString(GEOMETRY_SHADER, textdata->GetText());
                }
            }
            else if(param_name == "tess-cntl") {
                // TODO check for GL 4.0+
                // get source text
                auto textdata = resource::ResourceMap::Get<resource::TextFile>(param_val);
                if(textdata) {
                    LoadFromString(TESS_CONTROL_SHADER, textdata->GetText());
                }
            }
            else if(param_name == "tess-eval") {
                // TODO check for GL 4.0+
                // get source text
                auto textdata = resource::ResourceMap::Get<resource::TextFile>(param_val);
                if(textdata) {
                    LoadFromString(TESS_EVAL_SHADER, textdata->GetText());
                }
            }
            else if(param_name == "compute") {
                // TODO check for GL 4.4+
                // get source text
                auto textdata = resource::ResourceMap::Get<resource::TextFile>(param_val);
                if(textdata) {
                    LoadFromString(COMPUTE_SHADER, textdata->GetText());
                }
            }
        }
        else if(shade_param_itr->value.IsObject()) {
            if(param_name == "define") {
                for(auto sdef_itr = shade_param_itr->value.MemberBegin();
                        sdef_itr != shade_param_itr->value.MemberEnd(); sdef_itr++) {
                    std::string define_name(sdef_itr->name.GetString(), sdef_itr->name.GetStringLength());
                    if(sdef_itr->value.IsString()) {
                        std::string define_val(sdef_itr->name.GetString(), sdef_itr->name.GetStringLength());
                        // add a valued define
                    }
                    else if(sdef_itr->value.IsNull()) {
                        // add a blank define
                    }
                    else {
                        // invalid
                        // TODO use a logger
                        std::cerr << "[ERROR] Invalid shader define\n";
                        return false;
                    }
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
                        // TODO use a logger
                        std::cerr << "[ERROR] Invalid output definition\n";
                        return false;
                    }
                }
            }
            else {
                // TODO use a logger
                std::cerr << "[WARNING] Unknown shader parameter\n";
            }
        }
    }
    return true;
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
