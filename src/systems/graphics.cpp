#include "trillek-game.hpp"
#include "systems/graphics.hpp"
#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"

#include "resources/md5mesh.hpp"
#include "resources/transform.hpp"
#include "graphics/shader.hpp"
#include "graphics/material.hpp"
#include "components/renderable.hpp"

namespace trillek {
namespace graphics {

const int* RenderSystem::Start(const unsigned int width, const unsigned int height) {
    // Use the GL3 way to get the version number
    glGetIntegerv(GL_MAJOR_VERSION, &this->gl_version[0]);
    glGetIntegerv(GL_MINOR_VERSION, &this->gl_version[1]);

    // Sanity check to make sure we are at least in a good major version number.
    assert((this->gl_version[0] > 1) && (this->gl_version[0] < 5));

    SetViewportSize(width, height);

    // Set a default view that is back and up from the center.
    this->view_matrix = glm::lookAt(glm::vec3(0.0f, -10.0f, 4.0f),
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));

    // App specific global gl settings
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    return this->gl_version;
}

bool RenderSystem::Serialize(rapidjson::Document& document) {
    rapidjson::Value resource_node(rapidjson::kObjectType);

    document.AddMember("graphics", resource_node, document.GetAllocator());
    return true;
}

bool RenderSystem::DeSerialize(rapidjson::Value& node) {
    if(node.IsObject()) {
        // Iterate over types.
        for(auto type_itr = node.MemberBegin(); type_itr != node.MemberEnd(); ++type_itr) {
            std::string object_type(type_itr->name.GetString(), type_itr->name.GetStringLength());

            if(type_itr->value.IsObject()) {
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
    // Clear the backbuffer and primary depth/stencil buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, this->window_width, this->window_height); // Set the viewport size to fill the window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

    for (auto matgrp : this->material_groups) {
        const auto& shader = matgrp.material.GetShader();
        shader->Use();

        glUniformMatrix4fv((*shader)("view"), 1, GL_FALSE, &this->view_matrix[0][0]);
        glUniformMatrix4fv((*shader)("projection"), 1, GL_FALSE, &this->projection_matrix[0][0]);

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

                for (unsigned int entity_id : rengrp.instances) {
                    glUniformMatrix4fv((*shader)("model"), 1, GL_FALSE, &this->model_matrices.at(entity_id)[0][0]);
                    glDrawElements(GL_TRIANGLES, bufgrp->ibo_count, GL_UNSIGNED_INT, 0);
                }
            }
        }
    }
    TrillekGame::GetOS().SwapBuffers();
    // If the user closes the window, we notify all the systems
    if (TrillekGame::GetOS().Closing()) {
        TrillekGame::NotifyCloseWindow();
    }
}

void RenderSystem::Notify(const unsigned int entity_id, const transform::Transform* transform) {
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

void RenderSystem::AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component) {
    // Do a static_pointer_cast to make sure we do have a Renderable component.
    auto ren = std::static_pointer_cast<Renderable>(component);
    if (!ren) {
        return;
    }

    // Loop through all the renderables and see if one exists for the given entityID.
    for (auto& r : this->renderables) {
        if (r.first == entity_id) {
            r.second = ren;
            return;
        }
    }

    // No entry exists for the given entity ID, so add it.
    this->renderables.push_back(std::make_pair(entity_id, ren));

    MaterialGroup* matgrp = nullptr;
    // Check if the material for exists based on shader.
    // If it does make sure each one is using the same material.
    // TODO: Make material a resource-like element.
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

        const auto& shader = matgrp->material.GetShader();
        shader->Use();
        shader->AddUniform("view");
        shader->AddUniform("projection");
        shader->AddUniform("model");
        shader->UnUse();
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
                break;
            }
        }
        if (rengrp == nullptr) {
            MaterialGroup::TextureGroup::RenderableGroup temp;
            temp.renderable = ren;
            temp.buffer_group_index = i;
            temp.instances.push_back(entity_id);

            texgrp->renderable_groups.push_back(std::move(temp));
        }
    }

    // Subscribe to transform change events for this entity ID.
    event::Dispatcher<transform::Transform>::GetInstance()->Subscribe(entity_id, this);

    // We will use the notify method to force the initial model matrix creation.
    Notify(entity_id, transform::TransformMap::GetTransform(entity_id).get());
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

void RenderSystem::Terminate() {
    TrillekGame::GetOS().DetachContext();
}

} // End of graphics
} // End of trillek

