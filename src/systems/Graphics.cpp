#include "systems/Graphics.hpp"
#include "systems/ResourceSystem.h"

#include "resources/MD5Mesh.hpp"
#include "resources/Shader.hpp"
#include "resources/Material.hpp"
#include "components/Renderable.hpp"

namespace trillek {
namespace graphics {

const int* GL::Start(const unsigned int width, const unsigned int height) {
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

void GL::Update(const double delta) {
    // Clear the backbuffer and primary depth/stencil buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, this->window_width, this->window_height); // Set the viewport size to fill the window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

    for (auto& matgrp : this->material_groups) {
        const auto& shader = matgrp.material->GetShader();
        shader->Use();

        glUniformMatrix4fv((*shader)("view"), 1, GL_FALSE, &this->view_matrix[0][0]);
        glUniformMatrix4fv((*shader)("projection"), 1, GL_FALSE, &this->projection_matrix[0][0]);

        for (size_t i = 0; i < matgrp.renderables.size(); ++i) {
            matgrp.material->ActivateTexture(i, 0);
            for (auto& ren : matgrp.renderables[i]) {
                const auto& bufgrp = ren->GetBufferGroup(i);
                glBindVertexArray(bufgrp->vao);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufgrp->ibo);
                glDrawElements(GL_TRIANGLES, bufgrp->ibo_count, GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void GL::SetViewportSize(const unsigned int width, const unsigned int height) {
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

void GL::AddRenderable(const unsigned int entity_id, std::shared_ptr<Renderable> ren) {
    // Loop through all the renderables and see if one exists for the given entityID.
    for (auto& r : this->renderables) {
        if (r.first == entity_id) {
            r.second = ren;
            return;
        }
    }
            
    // No entry exists for the given entity ID, so add it.
    this->renderables.push_back(std::make_pair(entity_id, ren));

    // Check if there is a material group for the renderable's material.
    const auto& mat = ren->GetMaterial();
    if (!mat) {
        // TODO: Insert with a default material.
        return;
    }

    for (auto& matgrp : this->material_groups) {
        if (matgrp.material == mat) {
            // TODO: For each buffer group store the renderbale at the correct texture index.
            matgrp.renderables[0].push_back(ren);
        }
    }

    // There wasn't one, so add it.
    MaterialGroup matgrp;
    matgrp.material = mat;

    std::list<std::shared_ptr<Renderable>> renlist;
    renlist.push_back(ren);
    for (size_t i = 0; i < ren->GetBufferGroupCount(); ++i) {
        matgrp.renderables.push_back(renlist);
    }

    const auto& shader = mat->GetShader();
    shader->Use();
    shader->AddUniform("view");
    shader->AddUniform("projection");
    shader->UnUse();

    this->material_groups.push_back(std::move(matgrp));
}

void GL::RemoveRenderable(const unsigned int entity_id) {
    // Loop through all the renderables and see if one exists for the given entityID.
    for (auto& r : this->renderables) {
        if (r.first == entity_id) {
            for (auto& matgrp : this->material_groups) {
                for (auto& renlist : matgrp.renderables) {
                    renlist.remove(r.second);
                }
            }
            this->renderables.remove(r);
            return;
        }
    }
}

} // End of graphics
} // End of trillek

