#include "components/Renderable.hpp"
#include "resources/Material.hpp"
#include "resources/Mesh.hpp"

namespace trillek {
namespace graphics {
Renderable::Renderable() : mat(new resource::Material()) { }
Renderable::~Renderable() { }

void Renderable::UpdateBufferGroups() {
    // Check if the mesh is valid and assign an empty one if it isn't.
    if (!this->mesh) {
        return;
    }

    for (size_t i = 0; i < this->mesh->GetMeshGroupCount(); ++i) {
        std::shared_ptr<BufferGroup> bg;
        if (i < this->bufferGroups.size()) {
            bg = this->bufferGroups[i];
        }
        else {
            bg.reset(new BufferGroup());
            glGenVertexArrays(1, &bg->vao); // Generate the VAO
            glGenBuffers(1, &bg->vbo); 	// Generate the vertex buffer.
            glGenBuffers(1, &bg->ibo); // Generate the element buffer.
            this->bufferGroups.push_back(bg);
        }

        std::weak_ptr<resource::MeshGroup> mg = this->mesh->GetMeshGroup(i);
        glBindVertexArray(bg->vao); // Bind the VAO

        auto tempMG = mg.lock();

        if (tempMG) {
            if (tempMG->verts.size() > 0) {

                glBindBuffer(GL_ARRAY_BUFFER, bg->vbo); // Bind the vertex buffer.
                glBufferData(GL_ARRAY_BUFFER, sizeof(resource::VertexData) * tempMG->verts.size(), &tempMG->verts[0], GL_STATIC_DRAW); // Stores the verts in the vertex buffer.

                GLuint posLocation = 0; //glGetAttribLocation((*shader).GetProgram(), "in_Position"); // Find the location in the shader where the vertex buffer data will be placed.
                glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, resource::VertexData::position)); // Tell the VAO the vertex data will be stored at the location we just found.
                glEnableVertexAttribArray(posLocation); // Enable the VAO line for vertex data.

                GLuint colLocation = 2; //glGetAttribLocation((*shader).GetProgram(), "in_Color");
                glVertexAttribPointer(colLocation, 4, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, resource::VertexData::color));
                glEnableVertexAttribArray(colLocation);

                GLuint normalLocation = 1; //glGetAttribLocation((*shader).GetProgram(), "in_Normal");
                glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, resource::VertexData::normal));
                glEnableVertexAttribArray(normalLocation);

                GLuint uvLocation = 3; //glGetAttribLocation((*shader).GetProgram(), "in_UV"); // Find the location in the shader where the vertex buffer data will be placed.
                glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(resource::VertexData),
                    (GLvoid*)offsetof(resource::VertexData, resource::VertexData::uv)); // Tell the VAO the vertex data will be stored at the location we just found.
                glEnableVertexAttribArray(uvLocation); // Enable the VAO line for vertex data.
            }

            if (tempMG->indicies.size() > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg->ibo); // Bind the element buffer.
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* tempMG->indicies.size(), &tempMG->indicies[0], GL_STATIC_DRAW); // Store the faces in the element buffer.
                bg->iboCount = tempMG->indicies.size();
            }
        }

        glBindVertexArray(0); // Reset the buffer binding because we are good programmers.

    }
}

void Renderable::SetMesh(std::shared_ptr<resource::Mesh> m) {
    this->mesh = m;
}

std::shared_ptr<resource::Mesh> Renderable::GetMesh() {
    return this->mesh;
}

void Renderable::SetMaterial(std::shared_ptr<resource::Material> m) {
    this->mat = m;
}

std::shared_ptr<resource::Material> Renderable::GetMaterial() {
    return this->mat;
}

}
}
