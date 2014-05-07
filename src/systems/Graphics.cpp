#include "systems/Graphics.hpp"
#include "systems/ResourceSystem.h"

#include "resources/MD5Mesh.hpp"
#include "resources/Shader.hpp"
#include "resources/Material.hpp"
#include "components/Renderable.hpp"

#include <algorithm>


namespace trillek {
namespace graphics {
const int* GL::Start(const unsigned int width, const unsigned int height) {
    // Use the GL3 way to get the version number
    glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

    // Sanity check to make sure we are at least in a good major version number.
    assert((OpenGLVersion[0] > 1) && (OpenGLVersion[0] < 5));

    SetViewportSize(width, height);

    // Set a default view that is back and up from the center.
    this->view = glm::lookAt(glm::vec3(0.0f, -10.0f, 4.0f),
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));

    // App specific global gl settings
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    return OpenGLVersion;
}

void GL::Update(const double delta) {
    // Clear the backbuffer and primary depth/stencil buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, this->windowWidth, this->windowHeight); // Set the viewport size to fill the window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

    for (auto& matgrp : this->matGroups) {
        auto& shader = matgrp.mat->GetShader();
        shader->Use();

        glUniformMatrix4fv((*shader)("view"), 1, GL_FALSE, &this->view[0][0]);
        glUniformMatrix4fv((*shader)("projection"), 1, GL_FALSE, &this->proj[0][0]);
        for (size_t i = 0; i < matgrp.renderables.size(); ++i) {
            matgrp.mat->ActivateTexture(i, resource::Material::TEX0);
            for (auto& ren : matgrp.renderables[i]) {
                auto& bufgrp = ren->GetBufferGroup(i);
                glBindVertexArray(bufgrp->vao);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufgrp->ibo);
                glDrawElements(GL_TRIANGLES, bufgrp->iboCount, GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void GL::SetViewportSize(const unsigned int width, const unsigned int height) {
    this->windowHeight = height;
    this->windowWidth = width;

    // Determine the aspect ratio and sanity check it to a safe ratio
    float aspectRatio = static_cast<float>(this->windowWidth) / static_cast<float>(this->windowHeight);
    if (aspectRatio < 1.0f) {
        aspectRatio = 4.0f / 3.0f;
    }

    // update projection matrix based on new aspect ratio
    this->proj = glm::perspective(
        45.0f,
        aspectRatio,
        0.1f,
        10000.0f
        );
}

void GL::AddRenderable(const unsigned int entityID, std::shared_ptr<Renderable> ren) {
    // Loop through all the renderables and see if one exists for the given entityID.
    for (auto& r : this->renderables) {
        if (r.first == entityID) {
            r.second = ren;
            return;
        }
    }
            
    // No entry exists for the given entity ID, so add it.
    this->renderables.push_back(std::make_pair(entityID, ren));

    // Check if there is a material group for the renderable's material.
    auto& mat = ren->GetMaterial();
    if (!mat) {
        // TODO: Insert with a default material.
        return;
    }

    for (auto& matgrp : this->matGroups) {
        if (matgrp.mat == mat) {
            // TODO: For each buffer group store the renderbale at the correct texture index.
            matgrp.renderables[0].push_back(ren);
        }
    }

    // There wasn't one, so add it.
    MaterialGroup mg;
    mg.mat = mat;

    std::list<std::shared_ptr<Renderable>> renlist;
    renlist.push_back(ren);
    for (size_t i = 0; i < ren->GetBufferGroupCount(); ++i) {
        mg.renderables.push_back(renlist);
    }

    auto& shader = mat->GetShader();
    shader->Use();
    shader->AddUniform("view");
    shader->AddUniform("projection");
    shader->UnUse();

    this->matGroups.push_back(std::move(mg));
}

void GL::RemoveRenderable(const unsigned int entityID) {
    // Loop through all the renderables and see if one exists for the given entityID.
    for (auto& r : this->renderables) {
        if (r.first == entityID) {
            for (auto& matgrp : this->matGroups) {
                for (auto& renlist : matgrp.renderables) {
                    renlist.erase(std::remove_if(
                        renlist.begin(),
                        renlist.end(),
                        [r] (std::shared_ptr<Renderable> element) -> bool {
                            if (element == r.second) {
                                return true;
                            }
                            return false;
                        }),
                        renlist.end());
                }
            }
            this->renderables.remove(r);
            return;
        }
    }
}

}
}
