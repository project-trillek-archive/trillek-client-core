#include "transform.hpp"
#include "type-id.hpp"
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

namespace trillek {
namespace graphics {

RenderSystem::RenderSystem() : Parser("graphics") {
    multisample = false;
    Shader::InitializeTypes();
}

const int* RenderSystem::Start(const unsigned int width, const unsigned int height) {
    // Use the GL3 way to get the version number
    glGetIntegerv(GL_MAJOR_VERSION, &this->gl_version[0]);
    glGetIntegerv(GL_MINOR_VERSION, &this->gl_version[1]);
    CheckGLError();

    // Sanity check to make sure we are at least in a good major version number.
    assert((this->gl_version[0] > 1) && (this->gl_version[0] < 5));

    SetViewportSize(width, height);

    // Retrieve the default camera transform, and subscribe to changes to it.
    event::Dispatcher<Transform>::GetInstance()->Subscribe(0, this);

    // Activate the camera and get the initial view matrix.
    // TODO: Make camera into a component that is added to an entity.
    this->camera = std::make_shared<SixDOFCamera>();
    if (this->camera) {
        this->camera->Activate(0);
        this->view_matrix = this->camera->GetViewMatrix();
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
    settings.push_back(Property("version", gl_version[0] * 100 + gl_version[1] * 10));
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

bool RenderSystem::Serialize(rapidjson::Document& document) {
    rapidjson::Value resource_node(rapidjson::kObjectType);

    document.AddMember("graphics", resource_node, document.GetAllocator());
    return true;
}

bool RenderSystem::Parse(rapidjson::Value& node) {
    if(node.IsObject()) {
        // Iterate over types.
        for(auto type_itr = node.MemberBegin(); type_itr != node.MemberEnd(); ++type_itr) {
            std::string section_type(type_itr->name.GetString(), type_itr->name.GetStringLength());

            auto typefunc = parser_functions.find(section_type);
            if(typefunc != parser_functions.end()) {
                if(!typefunc->second(type_itr->value)) {
                    // TODO use logger
                    std::cerr << "[ERROR] Graphics parsing failed\n";
                    return false;
                }
            }
            else {
                // TODO use logger
                std::cerr << "[INFO] RenderSystem::Parse - skipping \"" << section_type << "\" section\n";
            }
        }
        return true;
    }
    return false;
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
            }
            else {
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

    RenderScene();

    TrillekGame::GetOS().SwapBuffers();
    // If the user closes the window, we notify all the systems
    if (TrillekGame::GetOS().Closing()) {
        TrillekGame::NotifyCloseWindow();
    }
}

void RenderSystem::RenderScene() const {

    glm::mat4x4 inv_proj = glm::inverse(this->projection_matrix);
    glViewport(0, 0, this->window_width, this->window_height);

    if(activerender) {
        for(auto& cmditem : activerender->render_commands) {
            if(!cmditem.resolved) {
                auto resolve = list_resolvers.find(cmditem.cmd);
                if(resolve != list_resolvers.end()) {
                    cmditem.run_values.clear();
                    if(!resolve->second(cmditem)) {
                        // should probably log some warning/error
                        // since this item may not be able to render
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
                switch(cmditem.run_values.front().Get<long>()) {
                case 0:
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glEnable(GL_DEPTH_TEST);
                    RenderColorPass(&this->view_matrix[0][0], &this->projection_matrix[0][0]);
                    break;
                case 1:
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glEnable(GL_DEPTH_TEST);
                    RenderDepthOnlyPass(&this->view_matrix[0][0], &this->projection_matrix[0][0]);
                    break;
                case 2:
                    glDisable(GL_MULTISAMPLE);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glDisable(GL_DEPTH_TEST);
                    RenderLightingPass(this->view_matrix, &inv_proj[0][0]);
                    break;
                case 3:
                    glDisable(GL_MULTISAMPLE);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glDisable(GL_DEPTH_TEST);
                    RenderPostPass();
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
                    }
                }
                else {
                    RenderLayer::UnbindFromAll();
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
                    src = ViewRect(0, 0, window_width, window_height);
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
                    dest = ViewRect(0, 0, window_width, window_height);
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

                static GLint temp;
                for (unsigned int entity_id : rengrp.instances) {
                    glUniformMatrix4fv((*shader)("model"), 1, GL_FALSE, &this->model_matrices.at(entity_id)[0][0]);
                    auto renanim = rengrp.animations.find(entity_id);
                    if (renanim != rengrp.animations.end()) {
                        temp = 1;
                        glUniform1iv((*shader)("animated"), 1, &temp);
                        auto &animmatricies = renanim->second->animation_matricies;
                        glUniformMatrix4fv((*shader)("animation_matrix"), animmatricies.size(), GL_FALSE, &animmatricies[0][0][0]);
                    }
                    else {
                        temp = 0;
                        glUniform1iv((*shader)("animated"), 1, &temp);
                    }
                    glDrawElements(GL_TRIANGLES, bufgrp->ibo_count, GL_UNSIGNED_INT, 0);
                }
            }
            for (size_t tex_index = 0; tex_index < texgrp.texture_indicies.size(); ++tex_index) {
                matgrp.material.DeactivateTexture(tex_index);
            }
        }
    }
}

void RenderSystem::RenderDepthOnlyPass(const float *view_matrix, const float *proj_matrix) const {
    // TODO Similar to color pass but without textures and everything uses a depth shader
    // This is intended for shadow map passes or the like
}

void RenderSystem::RenderLightingPass(const glm::mat4x4 &view_matrix, const float *inv_proj_matrix) const {
    glBindVertexArray(screenquad.vao); CheckGLError();
    GLint l_pos_loc = 0;
    GLint l_dir_loc = 0;
    GLint l_col_loc = 0;
    if(lightingshader) {
        lightingshader->Use();
        l_pos_loc = lightingshader->Uniform("light_pos");
        l_col_loc = lightingshader->Uniform("light_color");
        l_dir_loc = lightingshader->Uniform("light_dir");
        glUniform1i(lightingshader->Uniform("layer0"), 0);
        glUniform1i(lightingshader->Uniform("layer1"), 1);
        glUniform1i(lightingshader->Uniform("layer2"), 2);
        glUniform1i(lightingshader->Uniform("layer3"), 3);
        glUniformMatrix4fv(lightingshader->Uniform("inv_proj"), 1, GL_FALSE, inv_proj_matrix);
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    for (auto& clight : this->alllights) {
        if(clight.second && clight.second->enabled) {
            LightBase *activelight = clight.second.get();
            const glm::mat4& lightmat = this->model_matrices.at(clight.first);
            glm::vec4 lightpos = view_matrix * glm::vec4(lightmat[3][0], lightmat[3][1], lightmat[3][2], 1);
            glm::vec4 lightdir = glm::mat3x4(lightmat) * glm::vec3(0.f, 0.f, -1.f);
            if(l_pos_loc > 0) glUniform3f(l_pos_loc, lightpos.x, lightpos.y, lightpos.z);
            if(l_dir_loc > 0) glUniform3f(l_dir_loc, lightdir.x, lightdir.y, lightdir.z);
            if(l_col_loc > 0) glUniform3fv(l_col_loc, 1, (float*)&activelight->color);
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
            }
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0); // render quad for each light
        }
    }
    glDisable(GL_BLEND);
    // unbind when done
    glUseProgram(0);
    glBindVertexArray(0); CheckGLError();
}

void RenderSystem::RenderPostPass() const {

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
            }
        }
        return true;
    };
    parser_functions["settings"] = aglambda;
}

void RenderSystem::Notify(const unsigned int entity_id, const Transform* transform) {
    if (this->camera) {
        if (entity_id == this->camera->GetEntityID()) {
            this->view_matrix = this->camera->GetViewMatrix();
            return;
        }
    }
    glm::mat4 model_matrix = glm::translate(transform->GetTranslation()) *
        glm::mat4_cast(transform->GetOrientation()) *
        glm::scale(transform->GetScale());
    this->model_matrices[entity_id] = model_matrix;
}

void RenderSystem::SetViewportSize(const unsigned int width, const unsigned int height) {
    this->window_height = height;
    this->window_width = width;

    // Determine the aspect ratio and sanity check it to a safe ratio
    float aspect_ratio = static_cast<float>(this->window_width) / static_cast<float>(this->window_height);
    if (aspect_ratio < 1.0f) {
        aspect_ratio = 4.0f / 3.0f;
    }

    // update projection matrix based on new aspect ratio
    this->projection_matrix = glm::perspective(
        glm::radians(45.0f),
        aspect_ratio,
        0.1f,
        10000.0f
        );
}

template<>
bool RenderSystem::AddEntityComponent(const unsigned int entity_id, std::shared_ptr<LightBase> light) {

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
bool RenderSystem::AddEntityComponent(const unsigned int entity_id, std::shared_ptr<Renderable> ren) {

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
            for (size_t i = 0; i < tex_grp_itr.texture_indicies.size(),
                i < buffer_group->textures.size(); ++i) {
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

void RenderSystem::AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component) {

    int r;
    if(0 != (r = TryAddComponent<Renderable>(entity_id, component))) {
        if(r < 0) return;
    }
    else if(0 != (r = TryAddComponent<LightBase>(entity_id, component))) {
        if(r < 0) return;
    }

    // Subscribe to transform change events for this entity ID.
    event::Dispatcher<Transform>::GetInstance()->Subscribe(entity_id, this);

    // We will use the notify method to force the initial model matrix creation.
    Notify(entity_id, TransformMap::GetTransform(entity_id).get());
}

void RenderSystem::RemoveRenderable(const unsigned int entity_id) {
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

void RenderSystem::HandleEvents(const frame_tp& timepoint) {
    static frame_tp last_tp;
    std::chrono::duration<float> delta = timepoint - last_tp;
    last_tp = timepoint;
    for (auto ren : this->renderables) {
        if (ren.second->GetAnimation()) {
            ren.second->GetAnimation()->UpdateAnimation(delta.count());
        }
    }
};

void RenderSystem::Terminate() {
    TrillekGame::GetOS().DetachContext();
}

} // End of graphics
} // End of trillek

