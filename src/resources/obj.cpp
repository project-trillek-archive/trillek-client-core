#include "resources/obj.hpp"

#include <fstream>
#include <memory>
#include <sstream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace trillek {
namespace resource {

/**
 * \brief Cleans an input string by removing certain grouping characters.
 *
 * These characters include ", ', (, and ).
 * \param[in] std::string str The string to clean.
 * \return The cleaned string
 */
extern std::string CleanString(std::string str);

bool OBJ::ParseMTL(std::string fname) {
    std::ifstream f(fname, std::ios::in);

    std::string file_path;

    if (fname.find("/") != std::string::npos) {
        file_path = fname.substr(0, fname.find_last_of("/") + 1);
    }
    else if (fname.find("\"") != std::string::npos) {
        file_path = fname.substr(0, fname.find_last_of("\"") + 1);
    }

    if (!f.is_open()) {
        return false;
    }

    std::shared_ptr<MTL> currentMTL;

    std::string line;
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string identifier;

        ss >> identifier;
        if (identifier == "newmtl") {
            std::string name;
            ss >> name;
            this->materials[name] = std::make_shared<MTL>();
            currentMTL = this->materials[name];
        }
        else if (identifier == "Ka") {
            float r, g, b;
            ss >> r; ss >> g; ss >> b;
            currentMTL->ka[0] = r; currentMTL->ka[1] = g; currentMTL->ka[2] = b;
        }
        else if (identifier == "Kd") {
            float r, g, b;
            ss >> r; ss >> g; ss >> b;
            currentMTL->kd[0] = r; currentMTL->kd[1] = g; currentMTL->kd[2] = b;
        }
        else if (identifier == "Ks") {
            float r, g, b;
            ss >> r; ss >> g; ss >> b;
            currentMTL->ks[0] = r; currentMTL->ks[1] = g; currentMTL->ks[2] = b;
        }
        else if ((identifier == "Tr") || (identifier == "d")) {
            float tr;
            ss >> tr;
            currentMTL->tr = tr;
        }
        else if (identifier == "Ns") {
            float ns;
            ss >> ns;
            currentMTL->hardness = ns;
        }
        else if (identifier == "illum") {
            int i;
            ss >> i;
            currentMTL->illum = i;
        }
        else if (identifier == "map_Kd") {
            std::string filename;
            ss >> filename;
            currentMTL->diffuseMap = file_path + filename;
        }
        else if (identifier == "map_Ka") {
            std::string filename;
            ss >> filename;
            currentMTL->ambientMap = file_path + filename;
        }
        else if (identifier == "map_Bump") {
            std::string filename;
            ss >> filename;
            currentMTL->normalMap = file_path + filename;
        }
    }
    return true;
}

bool OBJ::Initialize(const std::vector<Property> &properties) {
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "filename") {
            this->fname = p.Get<std::string>();
        }
    }

    if (Parse()) {
        PopulateMeshGroups();
        return true;
    }

    return false;
}

bool OBJ::Parse() {
    std::ifstream f(this->fname, std::ios::in);

    std::string file_path;

    if (this->fname.find("/") != std::string::npos) {
        file_path = this->fname.substr(0, this->fname.find_last_of("/") + 1);
    }
    else if (this->fname.find("\"") != std::string::npos) {
        file_path = this->fname.substr(0, this->fname.find_last_of("\"") + 1);
    }

    if (!f.is_open()) {
        return false;
    }

    std::shared_ptr<VertexGroup> currentVGroup;

    std::string line;
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string identifier;

        ss >> identifier;
        if (identifier == "mtllib") {
            std::string fname;
            ss >> fname;
            ParseMTL(file_path + fname);
        }
        else if (identifier == "v") {
            glm::vec3 vert;
            ss >> vert.x; ss >> vert.y; ss >> vert.z;
            this->verticies.push_back(vert);
        }
        else if (identifier == "vt") {
            glm::vec2 uv;
            ss >> uv.x; ss >> uv.y;
            uv.y = 1 - uv.y;
            this->uvs.push_back(uv);
        }
        else if (identifier == "vn") {
            glm::vec3 norm;
            ss >> norm.x; ss >> norm.y; ss >> norm.z;
            this->normals.push_back(norm);
        }
        else if ((identifier == "g") || (identifier == "o")) {
            std::string name;
            ss >> name;
            if (currentVGroup) {
                this->vertexGroups.push_back(currentVGroup);
            }
            currentVGroup = std::make_shared<VertexGroup>();
            currentVGroup->name = name;
        }
        else if (identifier == "usemtl") {
            std::string mtlname;
            ss >> mtlname;
            if (currentVGroup) {
                currentVGroup->mtl = mtlname;
            }
        }
        else if (identifier == "f") {
            Triangle vert, uv, norm;
            std::string faceLine;
            std::getline(ss, faceLine);
            // Check if we have 3 vertex indicies per face vertex.
            if (faceLine.find("/") != std::string::npos) {
                // Check if the UV is ommited and replace it with -1 if it is.
                while (faceLine.find("//") != std::string::npos) {
                    faceLine = faceLine.replace(faceLine.find("//"), 2, " -1 ");
                }
                // Replace the / separators with spaces for stringstream ouput.
                std::replace(faceLine.begin(), faceLine.end(), '/', ' ');
                std::stringstream face_ss(faceLine);
                face_ss >> vert.verts[0]; face_ss >> uv.verts[0]; face_ss >> norm.verts[0];
                face_ss >> vert.verts[1]; face_ss >> uv.verts[1]; face_ss >> norm.verts[1];
                face_ss >> vert.verts[2]; face_ss >> uv.verts[2]; face_ss >> norm.verts[2];
            }
            else {
                std::stringstream face_ss(faceLine);
                // There is only 1 vertex index per face vertex.
                face_ss >> vert.verts[0]; face_ss >> vert.verts[1]; face_ss >> vert.verts[2];
                uv.verts[0] = -1; uv.verts[1] = -1; uv.verts[2] = -1;
                norm.verts[0] = -1; norm.verts[1] = -1; norm.verts[2] = -1;
            }
            if (currentVGroup) {
                currentVGroup->verts.push_back(vert);
                currentVGroup->uvs.push_back(uv);
                currentVGroup->norms.push_back(norm);
            }
        }
    }

    if (currentVGroup) {
        this->vertexGroups.push_back(currentVGroup);
    }

    return true;
}

void OBJ::PopulateMeshGroups() {
    for (auto vgroup : this->vertexGroups) {
        auto mesh = std::make_unique<Mesh>();
        mesh->shader = vgroup->mtl;
        for (size_t i = 0, j = 0; i < vgroup->verts.size(); ++i) {
            Triangle face;
            Vertex v0;
            v0.position = this->verticies[vgroup->verts[i].verts[0] - 1];
            v0.normal = this->normals[vgroup->norms[i].verts[0] - 1];
            v0.uv = this->uvs[vgroup->uvs[i].verts[0] - 1];
            mesh->verts.push_back(v0);
            face.verts[0] = j++;
            Vertex v1;
            v1.position = this->verticies[vgroup->verts[i].verts[1] - 1];
            v1.normal = this->normals[vgroup->norms[i].verts[1] - 1];
            v1.uv = this->uvs[vgroup->uvs[i].verts[1] - 1];
            mesh->verts.push_back(v1);
            face.verts[1] = j++;
            Vertex v2;
            v2.position = this->verticies[vgroup->verts[i].verts[2] - 1];
            v2.normal = this->normals[vgroup->norms[i].verts[2] - 1];
            v2.uv = this->uvs[vgroup->uvs[i].verts[2] - 1];
            mesh->verts.push_back(v2);
            face.verts[2] = j++;
            mesh->tris.push_back(face);
        }
        this->meshes.push_back(std::move(mesh));
    }

    if (this->mesh_groups.size() < this->meshes.size()) {
        this->mesh_groups.resize(this->meshes.size());
        for (auto& mgruop : this->mesh_groups) {
            if (!mgruop) {
                mgruop = std::make_shared<MeshGroup>();
            }
        }
    }

    for (size_t i = 0; i < this->meshes.size(); ++i) {
        if (this->mesh_groups[i]->verts.size() < this->meshes[i]->verts.size()) {
            this->mesh_groups[i]->verts.resize(this->meshes[i]->verts.size());
        }
        this->mesh_groups[i]->textures.push_back(this->materials[this->meshes[i]->shader]->diffuseMap);
        for (size_t j = 0; j < this->meshes[i]->verts.size(); ++j) {
            VertexData vdata;
            vdata.position = this->meshes[i]->verts[j].position;
            vdata.uv = this->meshes[i]->verts[j].uv;
            vdata.normal = this->meshes[i]->verts[j].normal;

            this->mesh_groups[i]->verts[j] = vdata;
        }
    }

    for (size_t i = 0; i < this->meshes.size(); ++i) {
        for (size_t j = 0; j < this->meshes[i]->tris.size(); ++j) {
            this->mesh_groups[i]->indicies.push_back(this->meshes[i]->tris[j].verts[0]);
            this->mesh_groups[i]->indicies.push_back(this->meshes[i]->tris[j].verts[1]);
            this->mesh_groups[i]->indicies.push_back(this->meshes[i]->tris[j].verts[2]);
        }
    }
}

} // End of resource
} // End of trillek
