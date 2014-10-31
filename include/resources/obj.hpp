#ifndef OBJ_HPP_INCLUDED
#define OBJ_HPP_INCLUDED

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "resources/mesh.hpp"

namespace trillek {
namespace resource {

class OBJ : public Mesh {
public:
    OBJ() { }
    ~OBJ() { }

    // OBJ Helper struct for face vertex data indicies.
    struct Face {
        Face() {
            this->pos[0] = 0; this->pos[1] = 0; this->pos[2] = 0;
            this->uv[0] = 0; this->uv[1] = 0; this->uv[2] = 0;
            this->norm[0] = 0; this->norm[1] = 0; this->norm[2] = 0;
        }
        unsigned int pos[3]; // index
        unsigned int uv[3]; // index
        unsigned int norm[3]; // index
    };

    // OBJ Helper struct for storing vertex groups
    struct VertexGroup {
        std::string name;
        std::string mtl;
        std::vector<Face> faces;
    };

    // MTL Helper struct describing mtl files
    struct MTL {
        glm::vec3 ka;
        glm::vec3 kd;
        glm::vec3 ks;
        float tr;
        float hardness;
        int illum;
        std::string ambientMap;
        std::string diffuseMap;
        std::string specularMap;
        std::string normalMap;
    };

    /**
     * \brief Returns a resource with the specified name.
     *
     * The only used initialization property is "filename".
     * \param[in] const std::vector<Property>& properties The creation properties for the resource.
     * \return bool True if initialization finished with no errors.
     */
    virtual bool Initialize(const std::vector<Property>& properties);

    /**
     * \brief Loads the OBJ file from disk and parses it.
     *
     * \return bool If the mesh was valid and loaded correctly.
     */
    bool Parse();

    /**
     * \brief Loads the MTL file from disk and parses it.
     *
     * \return bool If the material was valid and loaded correctly.
     */
    bool ParseMTL(std::string fname);

    /**
     * \brief Calculates the final vertex positions based on the bind-pose skeleton.
     *
     * There isn't a return as the processing will just do nothing if the
     * parse data was default objects.
     * \return void
     */
    void PopulateMeshGroups();

    /**
     * \brief Sets the mesh filename.
     *
     * This is just a shorthand function that can be called directly via script API.
     * \param[in] const std::string& fname The mesh filename.
     * \return bool True if initialization finished with no errors.
     */
    void SetFileName(const std::string& fname) {
        this->fname = fname;
    }
private:
    std::string fname; // Relative filename
    std::vector<std::shared_ptr<VertexGroup>> vertexGroups;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::map<std::string, std::shared_ptr<MTL>> materials;
};

} // End of resource

namespace reflection {

template <> inline const char* GetTypeName<resource::OBJ>() { return "OBJ"; }
template <> inline unsigned int GetTypeID<resource::OBJ>() { return 1001; }

} // End of reflection
} // End of trillek

#endif
