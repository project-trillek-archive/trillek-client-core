#include "components/Renderable.hpp"
#include "resources/Material.hpp"
#include "resources/Shader.hpp"
#include "resources/Mesh.hpp"

namespace trillek {
namespace graphics {

Renderable::Renderable() : material(std::make_shared<resource::Material>()) { }
Renderable::~Renderable() { }

void Renderable::UpdateBufferGroups() {
    // Check if the mesh is valid and assign an empty one if it isn't.
    if (!this->mesh) {
        return;
    }

    for (size_t i = 0; i < this->mesh->GetMeshGroupCount(); ++i) {
        std::shared_ptr<BufferGroup> buffer_group;
        if (i < this->buffer_groups.size()) {
            buffer_group = this->buffer_groups[i];
        }
        else {
            buffer_group = std::make_shared<BufferGroup>();
            glGenVertexArrays(1, &buffer_group->vao); // Generate the VAO
            glGenBuffers(1, &buffer_group->vbo); 	// Generate the vertex buffer.
            glGenBuffers(1, &buffer_group->ibo); // Generate the element buffer.
            this->buffer_groups.push_back(buffer_group);
        }

        std::weak_ptr<resource::MeshGroup> mesh_group = this->mesh->GetMeshGroup(i);
        glBindVertexArray(buffer_group->vao); // Bind the VAO

        auto temp_meshgroup = mesh_group.lock();

        if (temp_meshgroup) {
            if (temp_meshgroup->verts.size() > 0) {

                glBindBuffer(GL_ARRAY_BUFFER, buffer_group->vbo); // Bind the vertex buffer.
                glBufferData(GL_ARRAY_BUFFER, sizeof(resource::VertexData) * temp_meshgroup->verts.size(), &temp_meshgroup->verts[0], GL_STATIC_DRAW); // Stores the verts in the vertex buffer.

                GLuint shader_program = 0;
                if (this->material) {
                    shader_program = this->material->GetShader()->GetProgram();
                }

                GLuint posLocation = glGetAttribLocation(shader_program, "pos");
                glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, position)); // Tell the VAO the vertex data will be stored at the location we just found.
                glEnableVertexAttribArray(posLocation); // Enable the VAO line for vertex data.

                GLuint normalLocation = glGetAttribLocation(shader_program, "norm");
                glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, normal));
                glEnableVertexAttribArray(normalLocation);

                GLuint colLocation = glGetAttribLocation(shader_program, "color");
                glVertexAttribPointer(colLocation, 4, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, color));
                glEnableVertexAttribArray(colLocation);

                GLuint uvLocation = glGetAttribLocation(shader_program, "tex1");
                glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, uv)); // Tell the VAO the vertex data will be stored at the location we just found.
                glEnableVertexAttribArray(uvLocation); // Enable the VAO line for vertex data.
            }

            if (temp_meshgroup->indicies.size() > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_group->ibo); // Bind the element buffer.
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* temp_meshgroup->indicies.size(), &temp_meshgroup->indicies[0], GL_STATIC_DRAW); // Store the faces in the element buffer.
                buffer_group->ibo_count = temp_meshgroup->indicies.size();
            }
        }

        glBindVertexArray(0); // Reset the buffer binding because we are good programmers.

    }
}

void Renderable::SetMesh(std::shared_ptr<resource::Mesh> m) {
    this->mesh = m;
}

std::shared_ptr<resource::Mesh> Renderable::GetMesh() const {
    return this->mesh;
}

void Renderable::SetMaterial(std::shared_ptr<resource::Material> m) {
    this->material = m;
}

std::shared_ptr<resource::Material> Renderable::GetMaterial() const {
    return this->material;
}

} // End of graphics
} // End of trillek
