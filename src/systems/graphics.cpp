#include "transform.hpp"
#include "type-id.hpp"
#include "trillek-game.hpp"
#include "systems/graphics.hpp"
#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"
#include "resources/text-file.hpp"
#include "resources/md5mesh.hpp"
#include "resources/mesh.hpp"
#include "graphics/shader.hpp"
#include "graphics/material.hpp"
#include "graphics/renderable.hpp"
#include "graphics/six-dof-camera.hpp"
#include "graphics/animation.hpp"
#include "graphics/light.hpp"
#include "graphics/render-list.hpp"
#include "logging.hpp"

namespace trillek {
namespace graphics {

RenderSystem::RenderSystem() : Parser("graphics") {
    multisample = false;
    this->frame_drop = false;
    Shader::InitializeTypes();
}

const int* RenderSystem::Start(const unsigned int width, const unsigned int height) {
    // Use the GL3 way to get the version number
    glGetIntegerv(GL_MAJOR_VERSION, &this->gl_version[0]);
    glGetIntegerv(GL_MINOR_VERSION, &this->gl_version[1]);
    std::string glsl_string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    std::string glren_string((char*)glGetString(GL_RENDERER));
    std::string glver_string((char*)glGetString(GL_VERSION));
    //glGetIntegerv(GL_SHADING_LANGUAGE_VERSION, &this->gl_version[3]);
    CheckGLError();
    int opengl_version = gl_version[0] * 100 + gl_version[1] * 10;
    debugmode = 0;

    // Subscribe to events
    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);

    LOGMSGC(INFO) << "GLSL version " << glsl_string;
    LOGMSGC(INFO) << "OpenGL renderer " << glren_string;
    LOGMSGC(INFO) << "OpenGL version " << glver_string;
    if(opengl_version < 300) {
        LOGMSGC(FATAL) << "OpenGL context (" << opengl_version << ") less than required minimum (300)";
        assert(opengl_version >= 300);
    }
    if(opengl_version < 330) {
        LOGMSGC(WARNING) << "OpenGL context (" << opengl_version << ") less than recommended (330)";
    }
    else {
        LOGMSGC(INFO) << "OpenGL context (" << opengl_version << ')';
    }

    SetViewportSize(width, height);

    // copy the game transforms as graphic transforms
    component::OnTrue(component::Bitmap<component::Component::GameTransform>(),
        [&](id_t entity_id) {
            auto cp = component::Get<component::Component::GameTransform>(entity_id);
            component::Insert<component::Component::GraphicTransform>(entity_id, std::move(cp));
            LOGMSGC(INFO) << "Copying transform of entity " << entity_id;
        }
    );

    // Activate the lowest ID or first camera and get the initial view matrix.
    id_t cam_idnum = 0;
    std::weak_ptr<CameraBase> cam_ptr;
    auto cam_itr = cameras.begin();
    if(cam_itr != cameras.end()) {
        cam_idnum = cam_itr->first;
        cam_ptr = cam_itr->second;
        for(cam_itr++ ; cam_itr != cameras.end(); cam_itr++) {
            if(cam_itr->first < cam_idnum) {
                cam_idnum = cam_itr->first;
                cam_ptr = cam_itr->second;
            }
        }
        this->camera_id = cam_idnum;
        this->camera = cam_ptr.lock(); // get the ptr to it
    }
    else {
        LOGMSGC(INFO) << "No camera found, creating a camera id #0";
        // make one if none found
        this->camera_id = 0;
        this->camera = std::make_shared<SixDOFCamera>();
    }
    if (this->camera) {
        this->camera->Activate(cam_idnum);
        this->vp_center.view_matrix = this->camera->GetViewMatrix();
    }

    float quaddata[] = {
        -1,  1, 0, 1,
         1,  1, 1, 1,
        -1, -1, 0, 0,
         1, -1, 1, 0
    };
    uint16_t quadindicies[] = { 0, 2, 1, 1, 2, 3 };
    const unsigned QUADVERTSIZE = sizeof(float) * 4;
    glGenVertexArrays(1, &screenquad.vao); // Generate the VAO
    glGenBuffers(1, &screenquad.vbo); // Generate the vertex buffer.
    glGenBuffers(1, &screenquad.ibo); // Generate the element buffer.

    glBindVertexArray(screenquad.vao); CheckGLError(); // Bind the VAO

    glBindBuffer(GL_ARRAY_BUFFER, screenquad.vbo); CheckGLError(); // Bind the vertex buffer.

    // Store the verts in the buffer.
    glBufferData(GL_ARRAY_BUFFER, sizeof(quaddata), quaddata, GL_STATIC_DRAW); CheckGLError();

    // Set the layout for the shaders
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, QUADVERTSIZE, (GLvoid*)0);
    glEnableVertexAttribArray(0); CheckGLError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenquad.ibo); // Bind the element buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindicies), quadindicies, GL_STATIC_DRAW);
    CheckGLError();

    glBindVertexArray(0); CheckGLError(); // unbind VAO when done

    std::list<Property> settings;
    settings.push_back(Property("version", opengl_version));
    settings.push_back(Property("screen-width", width));
    settings.push_back(Property("screen-height", height));
    settings.push_back(Property("multisample", this->multisample));
    settings.push_back(Property("samples", (int)8));

    for(unsigned int p = 0; p < 3; p++) {
        for(auto& ginstance : this->graphics_instances) {
            for(auto& gobject : ginstance.second) {
                if(gobject.second && gobject.second->initialize_priority == p) {
                    gobject.second->SystemStart(settings);
                }
            }
        }
    }
    return this->gl_version;
}

bool RenderSystem::Parse(rapidjson::Value& node) {
    if(node.IsObject()) {
        // Iterate over types.
        for(auto type_itr = node.MemberBegin(); type_itr != node.MemberEnd(); ++type_itr) {
            std::string section_type(type_itr->name.GetString(), type_itr->name.GetStringLength());

            auto typefunc = parser_functions.find(section_type);
            if(typefunc != parser_functions.end()) {
                if(!typefunc->second(type_itr->value)) {
                    LOGMSGC(ERROR) << "Graphics parsing failed";
                    return false;
                }
            }
            else {
                LOGMSGC(INFO) << "RenderSystem::Parse - skipping \"" << section_type << "\" section";
            }
        }
        return true;
    }
    return false;
}

bool RenderSystem::Serialize(rapidjson::Document& document) {
    rapidjson::Value resource_node(rapidjson::kObjectType);

    document.AddMember("graphics", resource_node, document.GetAllocator());
    return true;
}

void RenderSystem::RegisterListResolvers() {
    std::map<std::string, GLuint> fbo_copytype_map;
    fbo_copytype_map["all"] = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    fbo_copytype_map["color"] = GL_COLOR_BUFFER_BIT;
    fbo_copytype_map["colour"] = GL_COLOR_BUFFER_BIT; // just because
    fbo_copytype_map["depth"] = GL_DEPTH_BUFFER_BIT;
    fbo_copytype_map["stencil"] = GL_STENCIL_BUFFER_BIT;
    fbo_copytype_map["depth-stencil"] = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;

    const RenderSystem& rensys = *this;
    list_resolvers[RenderCmd::CLEAR_SCREEN] = [&rensys] (RenderCommandItem &rlist) -> bool {
        return true;
    };
    list_resolvers[RenderCmd::MODULE_CMD] = [&rensys] (RenderCommandItem &rlist) -> bool {
        return true;
    };
    list_resolvers[RenderCmd::SCRIPT] = [&rensys] (RenderCommandItem &rlist) -> bool {
        return true;
    };
    list_resolvers[RenderCmd::RENDER] = [&rensys] (RenderCommandItem &rlist) -> bool {
        if(rlist.cmdvalue.Is<std::string>()) {
            const std::string &rentype = rlist.cmdvalue.Get<std::string>();
            if(rentype == "all-geometry") {
                rlist.run_values.push_back(Container((long)0));
            }
            else if(rentype == "depth-geometry") {
                rlist.run_values.push_back(Container((long)1));
            }
            else if(rentype == "lighting") {
                rlist.run_values.push_back(Container((long)2));
            }
            else if(rentype == "post") {
                rlist.run_values.push_back(Container((long)3));
                for(auto pitr = rlist.load_properties.begin(); pitr != rlist.load_properties.end(); pitr++) {
                    if(pitr->GetName() == "shader" && pitr->Is<std::string>()) {
                        auto shader_ptr = rensys.Get<Shader>(pitr->Get<std::string>());
                        if(shader_ptr) {
                            rlist.run_values.push_back(Container(shader_ptr));
                        }
                    }
                }
            }
            else {
                LOGMSGON(ERROR, rensys) << "Invalid render method";
                return false;
            }
        }
        else {
            return false;
        }
        return true;
    };
    list_resolvers[RenderCmd::SET_PARAM] = [&rensys] (RenderCommandItem &rlist) -> bool {
        return true;
    };
    auto layerresolver = [&rensys] (RenderCommandItem &rlist) -> bool {
        if(rlist.cmdvalue.IsEmpty()) {
            rlist.run_values.push_back(Container(false));
        }
        else if(rlist.cmdvalue.Is<std::string>()) {
            rlist.run_values.push_back(Container(true));
            auto layerptr = rensys.Get<RenderLayer>(rlist.cmdvalue.Get<std::string>());
            if(!layerptr) {
                LOGMSGON(ERROR, rensys) << "Layer not found: " << rlist.cmdvalue.Get<std::string>();
                return false;
            }
            rlist.run_values.push_back(Container(layerptr));
        }
        return true;
    };
    list_resolvers[RenderCmd::READ_LAYER] = layerresolver;
    list_resolvers[RenderCmd::WRITE_LAYER] = layerresolver;
    list_resolvers[RenderCmd::SET_RENDER_LAYER] = layerresolver;
    list_resolvers[RenderCmd::BIND_LAYER_TEXTURES] = layerresolver;
    list_resolvers[RenderCmd::COPY_LAYER] = [fbo_copytype_map, &rensys] (RenderCommandItem &rlist) -> bool {
        if(rlist.cmdvalue.IsEmpty()) {
            rlist.run_values.push_back(Container(false));
        }
        else if(rlist.cmdvalue.Is<std::string>()) {
            rlist.run_values.push_back(Container(true));
            auto layerptr = rensys.Get<RenderLayer>(rlist.cmdvalue.Get<std::string>());
            if(!layerptr) {
                LOGMSGON(ERROR, rensys) << "Layer not found: " << rlist.cmdvalue.Get<std::string>();
                return false;
            }
            rlist.run_values.push_back(Container(layerptr));
        }
        std::shared_ptr<RenderLayer> target;
        GLuint copytypebits = 0;
        for(auto& prop : rlist.load_properties) {
            if(prop.GetName() == "type") {
                if(prop.Is<std::string>()) {
                    const std::string& typestring = prop.Get<std::string>();
                    auto fboct = fbo_copytype_map.find(typestring);
                    if(fboct != fbo_copytype_map.end()) {
                        copytypebits |= fboct->second;
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return false;
                }
            }
            else if(prop.GetName() == "to") {
                if(prop.Is<std::string>()) {
                    target = rensys.Get<RenderLayer>(prop.Get<std::string>());
                    if(!target) {
                        LOGMSGON(ERROR, rensys) << "Layer not found: " << prop.Get<std::string>();
                        return false;
                    }
                }
            }
        }
        if(!target) {
            rlist.run_values.push_back(Container(false));
        }
        else {
            rlist.run_values.push_back(Container(true));
            rlist.run_values.push_back(Container(target));
        }
        rlist.run_values.push_back(Container(copytypebits));
        return true;
    };
    list_resolvers[RenderCmd::BIND_TEXTURE] = [&rensys] (RenderCommandItem &rlist) -> bool {
        return true;
    };
    list_resolvers[RenderCmd::BIND_SHADER] = [&rensys] (RenderCommandItem &rlist) -> bool {
        return true;
    };
}

void RenderSystem::ThreadInit() {
    TrillekGame::GetOS().MakeCurrent();
}

void RenderSystem::RunBatch() const {

    if(!this->frame_drop) {
        RenderScene();

        TrillekGame::GetOS().SwapBuffers();
    }
    // If the user closes the window, we notify all the systems
    if (TrillekGame::GetOS().Closing()) {
        TrillekGame::NotifyCloseWindow();
    }
}

void RenderSystem::RenderScene() const {

    const ViewMatrixSet *c_view;
    c_view = &vp_center;
    glm::mat4x4 inv_proj = glm::inverse(c_view->projection_matrix);
    glViewport(c_view->viewport.x, c_view->viewport.y, c_view->viewport.z, c_view->viewport.w);

    if(activerender) {
        for(auto texitem = dyn_textures.begin(); texitem != dyn_textures.end(); texitem++) {
            if(texitem->expired()) {
                // TODO: generate a remove request
            }
            else {
                auto texptr = texitem->lock();
                texptr->Update();
            }
        }
        for(auto& cmditem : activerender->render_commands) {
            if(!cmditem.resolved && !cmditem.resolve_error) {
                auto resolve = list_resolvers.find(cmditem.cmd);
                if(resolve != list_resolvers.end()) {
                    cmditem.run_values.clear();
                    if(!resolve->second(cmditem)) {
                        cmditem.resolve_error = true;
                        LOGMSGC(ERROR) << "Parsing render command failed";
                        break;
                    }
                    cmditem.resolved = true;
                }
                else {
                    break;
                }
            }
            switch(cmditem.cmd) {
            case RenderCmd::CLEAR_SCREEN:
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                // Clear required buffers
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                break;
            case RenderCmd::MODULE_CMD:
                break;
            case RenderCmd::SCRIPT:
                break;
            case RenderCmd::RENDER:
            {
                auto val_itr = cmditem.run_values.begin();
                switch(val_itr->Get<long>()) {
                case 0:
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glEnable(GL_DEPTH_TEST);
                    RenderColorPass(&c_view->view_matrix[0][0], &c_view->projection_matrix[0][0]);
                    break;
                case 1:
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glEnable(GL_DEPTH_TEST);
                    RenderDepthOnlyPass(&c_view->view_matrix[0][0], &c_view->projection_matrix[0][0]);
                    break;
                case 2:
                    glDisable(GL_MULTISAMPLE);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glDisable(GL_DEPTH_TEST);
                    RenderLightingPass(c_view->view_matrix, &inv_proj[0][0]);
                    break;
                case 3:
                {
                    std::shared_ptr<Shader> postshader;
                    glDisable(GL_MULTISAMPLE);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glDisable(GL_DEPTH_TEST);

                    for(val_itr++; val_itr != cmditem.run_values.end(); val_itr++) {
                        if(val_itr->Is<std::shared_ptr<Shader>>()) {
                            postshader = val_itr->Get<std::shared_ptr<Shader>>();
                        }
                    }
                    RenderPostPass(postshader);
                }
                    break;
                default:
                    break;
                }
            }
                break;
            case RenderCmd::SET_PARAM:
                break;
            case RenderCmd::READ_LAYER:
            {
                auto run_op = cmditem.run_values.begin();
                if(run_op->Get<bool>()) {
                    run_op++;
                    auto layer = run_op->Get<std::shared_ptr<RenderLayer>>();
                    if(layer) {
                        layer->BindToRead();
                    }
                }
                else {
                    RenderLayer::UnbindFromAll();
                }
            }
                break;
            case RenderCmd::WRITE_LAYER:
            {
                auto run_op = cmditem.run_values.begin();
                if(run_op->Get<bool>()) {
                    run_op++;
                    auto layer = run_op->Get<std::shared_ptr<RenderLayer>>();
                    if(layer) {
                        layer->BindToWrite();
                    }
                }
                else {
                    RenderLayer::UnbindFromAll();
                }
            }
                break;
            case RenderCmd::SET_RENDER_LAYER:
            {
                auto run_op = cmditem.run_values.begin();
                if(run_op->Get<bool>()) {
                    run_op++;
                    auto layer = run_op->Get<std::shared_ptr<RenderLayer>>();
                    if(layer) {
                        layer->BindToRender();
                        if(layer->IsCustomSize()) {
                            ViewRect sizeview;
                            layer->GetRect(sizeview);
                            glViewport(sizeview.x, sizeview.y,
                                sizeview.z, sizeview.w);
                        }
                        else {
                            glViewport(c_view->viewport.x, c_view->viewport.y,
                                c_view->viewport.z, c_view->viewport.w);
                        }
                    }
                }
                else {
                    RenderLayer::UnbindFromAll();
                    glViewport(c_view->viewport.x, c_view->viewport.y, c_view->viewport.z, c_view->viewport.w);
                }
            }
                break;
            case RenderCmd::BIND_LAYER_TEXTURES:
            {
                auto run_op = cmditem.run_values.begin();
                if(run_op->Get<bool>()) {
                    run_op++;
                    auto layer = run_op->Get<std::shared_ptr<RenderLayer>>();
                    if(layer) {
                        layer->BindTextures();
                    }
                }
                else {
                }
            }
                break;
            case RenderCmd::COPY_LAYER:
            {
                ViewRect src, dest;
                auto run_op = cmditem.run_values.begin();
                if(run_op->Get<bool>()) {
                    run_op++;
                    auto layer = run_op->Get<std::shared_ptr<RenderLayer>>();
                    if(layer) {
                        layer->BindToRead();
                        layer->GetRect(src);
                    }
                }
                else {
                    RenderLayer::UnbindFromRead();
                    src = c_view->viewport;
                }
                run_op++;
                if(run_op->Get<bool>()) {
                    run_op++;
                    auto layer = run_op->Get<std::shared_ptr<RenderLayer>>();
                    if(layer) {
                        layer->BindToWrite();
                        layer->GetRect(dest);
                    }
                }
                else {
                    RenderLayer::UnbindFromWrite();
                    dest = c_view->viewport;
                }
                run_op++;
                GLuint typebits = run_op->Get<GLuint>();
                glBlitFramebuffer(src.x, src.y, src.z, src.w, dest.x, dest.y, dest.z, dest.w, typebits, GL_NEAREST);
            }
                break;
            case RenderCmd::BIND_TEXTURE:
                break;
            case RenderCmd::BIND_SHADER:
                break;
            }
        }
    }
}

void RenderSystem::RenderColorPass(const float *view_matrix, const float *proj_matrix) const {
    for (auto matgrp : this->material_groups) {
        const auto& shader = matgrp.material.GetShader();
        shader->Use();

        glUniformMatrix4fv((*shader)("view"), 1, GL_FALSE, view_matrix);
        glUniformMatrix4fv((*shader)("projection"), 1, GL_FALSE, proj_matrix);
        GLint u_model_loc = shader->Uniform("model");
        GLint u_animatrix_loc = shader->Uniform("animation_matrix");
        GLint u_animate_loc = shader->Uniform("animated");

        for (const auto& texgrp : matgrp.texture_groups) {
            // Activate all textures for this texture group.
            for (size_t tex_index = 0; tex_index < texgrp.texture_indicies.size(); ++tex_index) {
                matgrp.material.ActivateTexture(texgrp.texture_indicies[tex_index], tex_index);
            }

            // Loop through each renderable group.
            for (const auto& rengrp : texgrp.renderable_groups) {
                const auto& bufgrp = rengrp.renderable->GetBufferGroup(rengrp.buffer_group_index);
                glBindVertexArray(bufgrp->vao);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufgrp->ibo);

                for (id_t entity_id : rengrp.instances) {
                    glUniformMatrix4fv(u_model_loc, 1, GL_FALSE, &this->model_matrices.at(entity_id)[0][0]);
                    auto renanim = rengrp.animations.find(entity_id);
                    if (renanim != rengrp.animations.end()) {
                        glUniform1i(u_animate_loc, 1);
                        auto &animmatricies = renanim->second->animation_matricies;
                        glUniformMatrix4fv(u_animatrix_loc, animmatricies.size(), GL_FALSE, &animmatricies[0][0][0]);
                    }
                    else {
                        glUniform1i(u_animate_loc, 0);
                    }
                    glDrawElements(GL_TRIANGLES, bufgrp->ibo_count, GL_UNSIGNED_INT, 0);
                }
            }
            for (size_t tex_index = 0; tex_index < texgrp.texture_indicies.size(); ++tex_index) {
                Material::DeactivateTexture(tex_index);
            }
        }

        shader->UnUse();
    }
}

void RenderSystem::RenderDepthOnlyPass(const float *view_matrix, const float *proj_matrix) const {
    // Similar to color pass but without textures and everything uses a depth shader
    // This is intended for shadow map passes or the like
    if(!depthpassshader) {
        return;
    }
    CheckGLError();
    depthpassshader->Use();
    CheckGLError();
    if(this->alllights.begin() == this->alllights.end()) {
        return;
    }
    auto lightitr = this->alllights.begin();
    LightBase *light = lightitr->second.get();
    if(light == nullptr) return;
    const glm::mat4x4& lightmat = this->model_matrices.at(lightitr->first);
    glm::vec3 lightpos = glm::vec3(lightmat[3][0], lightmat[3][1], lightmat[3][2]);
    glm::vec4 lightdir = glm::mat3x4(lightmat) * glm::vec3(0.f, 0.f, -1.f);
    glm::mat4x4 light_matrix =
        glm::perspective(3.1415f*0.5f, 1.f, 0.5f, 10000.f)
        * glm::lookAt(lightpos, lightpos-UP_VECTOR, FORWARD_VECTOR);
    glm::mat4x4 invlight_matrix = glm::inverse(light_matrix);
    CheckGLError();
    glUniform3f(depthpassshader->Uniform("light_pos"), lightpos.x, lightpos.y, lightpos.z);
    glUniformMatrix4fv(depthpassshader->Uniform("light_vp"), 1, GL_FALSE, (float*)&light_matrix);
    CheckGLError();
    if(light->shadows) {
        light->depthmatrix = light_matrix;
    }
    glDrawBuffer(GL_NONE);
    GLint u_model_loc = depthpassshader->Uniform("model");
    GLint u_animatrix_loc = depthpassshader->Uniform("animation_matrix");
    GLint u_animate_loc = depthpassshader->Uniform("animated");
    for (auto matgrp : this->material_groups) {
        for (const auto& texgrp : matgrp.texture_groups) {
            // Loop through each renderable group.
            for (const auto& rengrp : texgrp.renderable_groups) {
                const auto& bufgrp = rengrp.renderable->GetBufferGroup(rengrp.buffer_group_index);
                glBindVertexArray(bufgrp->vao);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufgrp->ibo);

                for (id_t entity_id : rengrp.instances) {
                    glUniformMatrix4fv(u_model_loc, 1, GL_FALSE, &this->model_matrices.at(entity_id)[0][0]);
                    auto renanim = rengrp.animations.find(entity_id);
                    if (renanim != rengrp.animations.end()) {
                        glUniform1i(u_animate_loc, 1);
                        auto &animmatricies = renanim->second->animation_matricies;
                        glUniformMatrix4fv(u_animatrix_loc, animmatricies.size(), GL_FALSE, &animmatricies[0][0][0]);
                    }
                    else {
                        glUniform1i(u_animate_loc, 0);
                    }
                    glDrawElements(GL_TRIANGLES, bufgrp->ibo_count, GL_UNSIGNED_INT, 0);
                }
            }
        }
    }
    CheckGLError();
    Shader::UnUse();
    CheckGLError();
}

void RenderSystem::RenderLightingPass(const glm::mat4x4 &view_matrix, const float *inv_proj_matrix) const {
    glBindVertexArray(screenquad.vao); CheckGLError();
    GLint l_pos_loc = 0;
    GLint l_dir_loc = 0;
    GLint l_col_loc = 0;
    GLint l_type_loc = 0;
    GLint l_ushadow_loc = 0;
    GLint l_tshadow_loc = 0;
    GLint l_sshadow_loc = 0;
    if(lightingshader) {
        lightingshader->Use();
        l_pos_loc = lightingshader->Uniform("light_pos");
        l_col_loc = lightingshader->Uniform("light_color");
        l_dir_loc = lightingshader->Uniform("light_dir");
        l_type_loc = lightingshader->Uniform("light_type");
        l_ushadow_loc = lightingshader->Uniform("shadow_enabled");
        l_tshadow_loc = lightingshader->Uniform("shadow_matrix");
        l_sshadow_loc = lightingshader->Uniform("shadow_depth");
        glUniform1i(lightingshader->Uniform("layer0"), 0);
        glUniform1i(lightingshader->Uniform("layer1"), 1);
        glUniform1i(lightingshader->Uniform("layer2"), 2);
        glUniform1i(lightingshader->Uniform("layer3"), 3);
        if(l_sshadow_loc > 0) glUniform1i(l_sshadow_loc, 4);
        glUniformMatrix4fv(lightingshader->Uniform("inv_proj"), 1, GL_FALSE, inv_proj_matrix);
    }
    else {
        return;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    for (auto& clight : this->alllights) {
        if(clight.second && clight.second->enabled) {
            LightBase *activelight = clight.second.get();
            std::shared_ptr<Texture> shadowbuf;
            GLint useshadow = 0;
            const glm::mat4& lightmat = this->model_matrices.at(clight.first);
            glm::vec4 lightpos = view_matrix * glm::vec4(lightmat[3][0], lightmat[3][1], lightmat[3][2], 1);
            glm::vec4 lightdir = glm::mat3x4(lightmat) * glm::vec3(0.f, 0.f, -1.f);
            if(l_pos_loc > 0) glUniform3f(l_pos_loc, lightpos.x, lightpos.y, lightpos.z);
            if(l_dir_loc > 0) glUniform3f(l_dir_loc, lightdir.x, lightdir.y, lightdir.z);
            if(l_col_loc > 0) glUniform3fv(l_col_loc, 1, (float*)&activelight->color);
            if(l_type_loc > 0) glUniform1ui(l_type_loc, activelight->lighttype);
            auto lp_itr = activelight->light_props.begin();
            for(;lp_itr != activelight->light_props.end(); lp_itr++) {
                GLint uniformloc = lightingshader->Uniform(lp_itr->GetName().c_str());
                if(uniformloc > 0) {
                    if(lp_itr->Is<float>()) {
                        glUniform1f(uniformloc, lp_itr->Get<float>());
                    }
                    else if(lp_itr->Is<glm::vec3>()) {
                        glm::vec3 val = lp_itr->Get<glm::vec3>();
                        glUniform3f(uniformloc, val.x, val.y, val.z);
                    }
                    else if(lp_itr->Is<glm::vec4>()) {
                        glm::vec4 val = lp_itr->Get<glm::vec4>();
                        glUniform4f(uniformloc, val.x, val.y, val.z, val.w);
                    }
                    else if(lp_itr->Is<glm::vec2>()) {
                        glm::vec2 val = lp_itr->Get<glm::vec2>();
                        glUniform2f(uniformloc, val.x, val.y);
                    }
                }
                else if(activelight->shadows && lp_itr->GetName() == "shadow") {
                    shadowbuf = TrillekGame::GetGraphicSystem().Get<Texture>(lp_itr->Get<std::string>());
                    if(shadowbuf) {
                        useshadow = 1 + debugmode;
                        glActiveTexture(GL_TEXTURE4);
                        glBindTexture(GL_TEXTURE_2D, shadowbuf->GetID());
                        glm::mat4x4 invviewshadow = activelight->depthmatrix * glm::inverse(view_matrix);
                        if(l_tshadow_loc > 0) glUniformMatrix4fv(l_tshadow_loc, 1, GL_FALSE, &invviewshadow[0][0]);
                    }
                }
            }
            if(l_ushadow_loc > 0) glUniform1i(l_ushadow_loc, useshadow);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0); // render quad for each light
        }
    }
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    // unbind when done
    glUseProgram(0);
    glBindVertexArray(0); CheckGLError();
}

inline void RenderSystem::UpdateModelMatrices(const frame_tp& timepoint) {
    auto& transform_container = TrillekGame::GetSharedComponent()
                                                .Map<Component::GraphicTransform>();
    // since the data is published by the same thread, get the last published data
    // first remove the transforms
    auto& transform_map_neg = transform_container.GetLastNegativeCommit();
    for (const auto& transform : transform_map_neg) {
        this->model_matrices.erase(transform.first);
    }
    // second add the new ones
    auto& transform_map_pos = transform_container.GetLastPositiveCommit();
    // for each frame
    for (const auto& transform_el : transform_map_pos) {
        // for each modified transform in the frame
        const auto id = transform_el.first;
        const auto& transform = *component::Get<Component::GraphicTransform>(transform_el.second);
        glm::mat4 model_matrix = glm::translate(transform.GetTranslation()) *
            glm::mat4_cast(transform.GetOrientation()) *
            glm::scale(transform.GetScale());
        this->model_matrices[id] = std::move(model_matrix);
    }
    // Update the view matrix if necessary
    if (transform_map_pos.count(this->GetActiveCameraID())) {
        auto camera_transform = component::GetConstSharedPtr<Component::GraphicTransform>(this->GetActiveCameraID());
        this->camera->UpdateTransform(std::move(camera_transform));
        this->vp_center.view_matrix = this->camera->GetViewMatrix();
    }
}

void RenderSystem::RenderPostPass(std::shared_ptr<Shader> postshader) const {
    postshader->Use();
    glBindVertexArray(screenquad.vao); CheckGLError();
    glUniform1i(postshader->Uniform("layer0"), 0);CheckGLError();
    glUniform1i(postshader->Uniform("layer1"), 1);CheckGLError();
    glUniform1i(postshader->Uniform("layer2"), 2);CheckGLError();
    glUniform1i(postshader->Uniform("layer3"), 3);CheckGLError();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);CheckGLError();
    glBindVertexArray(0); CheckGLError();
    Shader::UnUse();
}

void RenderSystem::RegisterStaticParsers() {
    RenderSystem &rensys = *this;
    auto aglambda =  [&rensys] (const rapidjson::Value& node) -> bool {
        for(auto settingitr = node.MemberBegin(); settingitr != node.MemberEnd(); settingitr++) {
            std::string settingname = util::MakeString(settingitr->name);
            if(settingitr->value.IsString()) {
                std::string settingval = util::MakeString(settingitr->value);
                if(settingname == "active-graph") {
                    rensys.activerender = rensys.Get<RenderList>(settingval);
                }
                else if(settingname == "lighting-shader") {
                    rensys.lightingshader = rensys.Get<Shader>(settingval);
                }
                else if(settingname == "depth-shader") {
                    rensys.depthpassshader = rensys.Get<Shader>(settingval);
                }
            }
        }
        return true;
    };
    parser_functions["settings"] = aglambda;
}

void RenderSystem::SetViewportSize(const unsigned int width, const unsigned int height) {
    this->window_height = height;
    this->window_width = width;

    this->vp_center.viewport = ViewRect(0,0,width,height);

    // Determine the aspect ratio and sanity check it to a safe ratio
    float aspect_ratio = static_cast<float>(this->window_width) / static_cast<float>(this->window_height);
    if (aspect_ratio < 1.0f) {
        aspect_ratio = 4.0f / 3.0f;
    }

    // update projection matrix based on new aspect ratio
    this->vp_center.projection_matrix = glm::perspective(
        glm::radians(45.0f),
        aspect_ratio,
        0.1f,
        10000.0f
        );
}

template<>
void RenderSystem::Add(const std::string & instancename, std::shared_ptr<Texture> instanceptr) {
    unsigned int type_id = reflection::GetTypeID<Texture>();
    if(instanceptr->IsDynamic()) {
        dyn_textures.push_back(instanceptr);
    }
    graphics_instances[type_id][instancename] = instanceptr;
}

template<>
bool RenderSystem::AddEntityComponent(const id_t entity_id, std::shared_ptr<LightBase> light) {

    // Loop through all the lights and see if one exists for the given entity.
    for (auto& r : this->alllights) {
        if (r.first == entity_id) {
            r.second = light;
            return false;
        }
    }

    // No entry exists for the given entity, so add it.
    this->alllights.push_back(std::make_pair(entity_id, light));

    return true;
}

template<>
bool RenderSystem::AddEntityComponent(const id_t entity_id, std::shared_ptr<CameraBase> cam) {
    auto cam_itr = this->cameras.find(entity_id);
    if(cam_itr != this->cameras.end()) {
        this->cameras[entity_id] = cam; // replace existing
        return false;
    }
    this->cameras[entity_id] = cam;
    return true;
}

template<>
bool RenderSystem::AddEntityComponent(const id_t entity_id, std::shared_ptr<Renderable> ren) {

    // Loop through all the renderables and see if one exists for the given entity_id.
    for (auto& r : this->renderables) {
        if (r.first == entity_id) {
            r.second = ren;
            return false;
        }
    }

    // No entry exists for the given entity ID, so add it.
    this->renderables.push_back(std::make_pair(entity_id, ren));

    MaterialGroup* matgrp = nullptr;
    // Check if the material for exists based on shader.
    for (auto& mg : this->material_groups) {
        if (mg.material.GetShader() == ren->GetShader()) {
            matgrp = &mg;
        }
    }

    // There wasn't an existing material group so add one.
    if (matgrp == nullptr) {
        MaterialGroup mg;
        this->material_groups.push_back(mg);
        matgrp = &this->material_groups.back();

        matgrp->material.SetShader(ren->GetShader());

        //const auto& shader = matgrp->material.GetShader();
        //shader->Use();
        //shader->UnUse();
    }

    // Map the renderable into the render graph material groups.
    for (size_t i = 0; i < ren->GetBufferGroupCount(); ++i) {
        MaterialGroup::TextureGroup* texgrp = nullptr;

        auto buffer_group = ren->GetBufferGroup(i);

        for (MaterialGroup::TextureGroup& tex_grp_itr : matgrp->texture_groups) {
            if (texgrp != nullptr) {
                break;
            }
            // Loop through and see if all the texture indicies line up.
            for (size_t i = 0; (i < tex_grp_itr.texture_indicies.size()) &&
                (i < buffer_group->textures.size()); ++i) {
                if (tex_grp_itr.texture_indicies[i] != matgrp->material.GetTextureIndex(buffer_group->textures[i])) {
                    break;
                }
                texgrp = &tex_grp_itr;
            }
        }

        if (texgrp == nullptr) {
            MaterialGroup::TextureGroup temp;
            matgrp->texture_groups.push_back(std::move(temp));
            texgrp = &matgrp->texture_groups.back();

            // Loop through and add all the texture indicies.
            for (size_t i = 0; i < buffer_group->textures.size(); ++i) {
                texgrp->texture_indicies.push_back(matgrp->material.AddTexture(buffer_group->textures[i]));
            }
            //texgrp->texture_indicies.push_back(0);
        }

        MaterialGroup::TextureGroup::RenderableGroup* rengrp = nullptr;
        // If we made it then add entity instances based on the meshes being the same.
        for (MaterialGroup::TextureGroup::RenderableGroup& ren_grp_itr : texgrp->renderable_groups) {
            if ((ren_grp_itr.renderable->GetMesh() == ren->GetMesh()) && (ren_grp_itr.buffer_group_index == i)) {
                rengrp = &ren_grp_itr;
                rengrp->instances.push_back(entity_id);
                if (ren->GetAnimation()) {
                    rengrp->animations[entity_id] = ren->GetAnimation();
                }
                break;
            }
        }
        if (rengrp == nullptr) {
            MaterialGroup::TextureGroup::RenderableGroup temp;
            temp.renderable = ren;
            if (ren->GetAnimation()) {
                temp.animations[entity_id] = ren->GetAnimation();
            }

            temp.buffer_group_index = i;
            temp.instances.push_back(entity_id);

            texgrp->renderable_groups.push_back(std::move(temp));
        }
    }
    return true;
}

void RenderSystem::AddDynamicComponent(const id_t entity_id, std::shared_ptr<Container> component) {
    int r;
    if(0 != (r = TryAddComponent<Renderable>(entity_id, component))) {
        if(r < 0) {
            LOGMSGC(ERROR) << "Could not add component Renderable";
            return;
        }
    }
    else if(0 != (r = TryAddComponent<LightBase>(entity_id, component))) {
        if(r < 0) {
            LOGMSGC(ERROR) << "Could not add component LightBase";
            return;
        }
    }
    else if(0 != (r = TryAddComponent<CameraBase>(entity_id, component))) {
        if(r < 0) {
            LOGMSGC(ERROR) << "Could not add component CameraBase";
            return;
        }
    }
}

void RenderSystem::RemoveRenderable(const id_t entity_id) {
    // Loop through all the renderables and see if one exists for the given entityID.
    for (auto& r : this->renderables) {
        if (r.first == entity_id) {
            auto matgrp_itr = this->material_groups.begin();
            while (matgrp_itr != this->material_groups.end()) {
                auto texgrp_itr = matgrp_itr->texture_groups.begin();
                while (texgrp_itr != matgrp_itr->texture_groups.end()) {

                    auto rengrp_itr = texgrp_itr->renderable_groups.begin();
                    while (rengrp_itr != texgrp_itr->renderable_groups.end()) {
                        // Check if the renderblae is the one we are looking for an remove it.
                        if (rengrp_itr->renderable == r.second) {
                            rengrp_itr = texgrp_itr->renderable_groups.erase(rengrp_itr);
                        }
                        else {
                            ++rengrp_itr;
                        }
                    }

                    // Check if the texture group is empty and remove it from the list.
                    if (texgrp_itr->renderable_groups.size() == 0) {
                        texgrp_itr = matgrp_itr->texture_groups.erase(texgrp_itr);
                    }
                    else {
                        ++texgrp_itr;
                    }
                }

                // Check if the material group is empty and remove it from the list.
                if (matgrp_itr->texture_groups.size() == 0) {
                    matgrp_itr = this->material_groups.erase(matgrp_itr);
                }
                else {
                    ++matgrp_itr;
                }
            }
            this->renderables.remove(r);
            return;
        }
    }
}

void RenderSystem::HandleEvents(frame_tp timepoint) {
    auto now = TrillekGame::GetOS().GetTime().count();
    static frame_tp last_tp = now;
    auto delta = now - last_tp;
    if(delta > 66666666ll) {
        if(!this->frame_drop) {
            LOGMSGC(INFO) << "Time lag " << (delta - 16666666) * 1.0E-9 << " > 50 milliseconds";
            this->frame_drop_count = 0;
        }
        this->frame_drop = true;
        this->frame_drop_count++;
    }
    else {
        if(this->frame_drop) {
            LOGMSGC(INFO) << "Dropped frames " << this->frame_drop_count;
            this->frame_drop_count = 0;
        }
        this->frame_drop = false;
    }
    last_tp = now;
    for (auto ren : this->renderables) {
        if (ren.second->GetAnimation()) {
            ren.second->GetAnimation()->UpdateAnimation(delta * 1E-9);
        }
    }
    UpdateModelMatrices(timepoint);
};

void RenderSystem::Terminate() {
    TrillekGame::GetOS().DetachContext();
}

} // End of graphics
} // End of trillek

