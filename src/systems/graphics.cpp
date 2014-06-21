#include "trillek-game.hpp"
#include "transform.hpp"
#include "systems/graphics.hpp"
#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"
#include "resources/text-file.hpp"
#include "resources/md5mesh.hpp"
#include "transform.hpp"
#include "resources/mesh.hpp"
#include "graphics/shader.hpp"
#include "graphics/material.hpp"
#include "graphics/render-layer.hpp"
#include "graphics/renderable.hpp"
#include "graphics/six-dof-camera.hpp"
#include "graphics/animation.hpp"

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

            if(type_itr->value.IsObject()) {
                if(section_type == reflection::GetTypeName<Shader>()) {
                    for(auto shade_itr = type_itr->value.MemberBegin();
                            shade_itr != type_itr->value.MemberEnd(); shade_itr++) {
                        std::string shader_name(shade_itr->name.GetString(), shade_itr->name.GetStringLength());
                        if(shade_itr->value.IsObject()) {
                            std::shared_ptr<Shader> shade_ptr(new Shader);
                            if(shade_ptr->Parse(shader_name, shade_itr->value)) {
                                if(shade_ptr->LinkProgram()) {
                                    Add(shader_name, shade_ptr);
                                }
                            }
                        }
                        else {
                            // TODO use logger
                            std::cerr << "[WARNING] Invalid shader entry\n";
                        }
                    }
                }
                else if(section_type == reflection::GetTypeName<RenderAttachment>()) {
                    // TODO these sections are basically the same, and could use a more generic parse
                    for(auto section_itr = type_itr->value.MemberBegin();
                            section_itr != type_itr->value.MemberEnd(); section_itr++) {
                        std::string attachment_name(section_itr->name.GetString(), section_itr->name.GetStringLength());
                        if(section_itr->value.IsObject()) {
                            std::shared_ptr<RenderAttachment> attachment_ptr(new RenderAttachment);
                            if(attachment_ptr->Parse(attachment_name, section_itr->value)) {
                                Add(attachment_name, attachment_ptr);
                            }
                        }
                        else {
                            // TODO use logger
                            std::cerr << "[WARNING] Invalid attachment entry\n";
                        }
                    }
                }
                else if(section_type == reflection::GetTypeName<RenderLayer>()) {
                    for(auto section_itr = type_itr->value.MemberBegin();
                            section_itr != type_itr->value.MemberEnd(); section_itr++) {
                        std::string layer_name(section_itr->name.GetString(), section_itr->name.GetStringLength());
                        if(section_itr->value.IsObject()) {
                            std::shared_ptr<RenderLayer> layer_ptr(new RenderLayer);
                            if(layer_ptr->Parse(layer_name, section_itr->value)) {
                                Add(layer_name, layer_ptr);
                            }
                        }
                        else {
                            // TODO use logger
                            std::cerr << "[WARNING] Invalid render layer entry\n";
                        }
                    }
                }
            }
        }
        return true;
    }
    return false;
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

    glm::mat4x4 inv_viewproj = glm::inverse(this->projection_matrix * this->view_matrix);
    // Clear the backbuffer and primary depth/stencil buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, this->window_width, this->window_height); // Set the viewport size to fill the window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    RenderColorPass(&this->view_matrix[0][0], &this->projection_matrix[0][0]);

    glDisable(GL_MULTISAMPLE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);

    RenderLightingPass(&inv_viewproj[0][0]);
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

void RenderSystem::RenderLightingPass(const float *inv_viewproj_matrix) const {
    for (auto clight : this->alllights) {
        if(clight.second && clight.second->enabled) {
            LightBase *activelight = clight.second.get();
            const float *lighttransform = &this->model_matrices.at(clight.first)[0][0];
        }
    }
}

void RenderSystem::RenderPostPass() const {

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

    if(reflection::GetTypeID<Renderable>() == component->component_type_id) {
        // Do a static_pointer_cast to make sure we do have a Renderable component.
        auto ren = std::static_pointer_cast<Renderable>(component);
        if (!ren) {
            return;
        }
        if(!AddEntityComponent(entity_id, ren)) {
            return;
        }
    }
    else if(reflection::GetTypeID<LightBase>() == component->component_type_id) {
        auto light = std::static_pointer_cast<LightBase>(component);
        if (!light) {
            return;
        }
        if(!AddEntityComponent(entity_id, light)) {
            return;
        }
    }

    // TODO: should really only subscribe once per entity, not for each component on one entity ID
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

