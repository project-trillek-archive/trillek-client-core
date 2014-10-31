#ifndef MD5ANIM_HPP_INCLUDED
#define MD5ANIM_HPP_INCLUDED

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "systems/resource-system.hpp"

namespace trillek {
namespace resource {

class Mesh;

class MD5Anim : public ResourceBase {
public:
    MD5Anim() { }
    ~MD5Anim() { }

    /*****************************/
    /* MD5Anim helper structures */
    /*****************************/
    struct Joint {
        std::string name; // The name of the joint
        int parent; // index
        int flags;
        int start_index;
        glm::vec3 base_position;
        glm::quat base_orientation;
        glm::mat4 bind_pose_inverse;
        void ComputeW();
    };

    struct BoundingBox {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct SkeletonJoint {
        int parent;
        glm::vec3 position;
        glm::quat orientation;
        void ComputeW();
    };

    struct FrameSkeleton {
        std::vector<SkeletonJoint> skeleton_joints;
        std::vector<glm::mat4> bone_matricies;
    };

    struct Frame {
        int index;
        std::vector<float> parameters;
        FrameSkeleton skeleton;
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
    * \brief Loads the MD5Anim file from disk and parses it.
    *
    * \return bool If the animation was valid and loaded correctly.
    */
    bool Parse();

    /**
    * \brief Sets the animation filename.
    *
    * This is just a shorthand function that can be called directly via script API.
    * \param[in] const std::string& fname The mesh filename.
    * \return bool True if initialization finished with no errors.
    */
    void SetFileName(const std::string& fname) {
        this->fname = fname;
    }

    /**
    * \brief Returns the number of animation frames.
    *
    * \return bool size_t The number of frames.
    */
    size_t GetFrameCount() const {
        return this->frames.size();
    }

    /**
    * \brief Returns the number of frame rate per second.
    *
    * \return bool size_t The frame rate per second.
    */
    int GetFrameRate() const {
        return this->frame_rate;
    }

    /**
    * \brief Builds or rebuilds the FrameSkeleton for a given frame index.
    *
    * \param[in] size_t frame_index Index of the frame to build/rebuild.
    * \return void
    */
    void BuildFrameSkeleton(size_t frame_index);

    /**
    * \brief Checks of a given mesh is valid for this animation.
    *
    * \param[in] std::shared_ptr<Mesh> mesh_file The mesh to check against.
    * \return bool True if the mesh is compatible.
    */
    bool CheckMesh(std::shared_ptr<Mesh> mesh_file);

    /**
    * \brief Gets the interpolated FrameSkeleton between 2 frames at a given delta.
    *
    * \param[in] size_t frame_index_start The starting frame index.
    * \param[in] size_t frame_index_start The ending frame index.
    * \param[in] float delta The change in time since the last call.
    * \return FrameSkeleton The current FrameSkeleton for the given delta.
    */
    FrameSkeleton InterpolateSkeletons(size_t frame_index_start, size_t frame_index_end, float delta);
private:
    std::string fname; // Relative filename
    std::vector<std::unique_ptr<BoundingBox>> bounds; // Bound box sizes for each join.
    std::vector<std::unique_ptr<Frame>> frames;
    std::vector<std::unique_ptr<Joint>> joints;
    int num_joints; // Number of joints, from the file.
    int frame_rate; // Frame rate.
    int num_frames; // Number of frames, from the file.
};

} // End of resource

namespace reflection {

template <> inline const char* GetTypeName<resource::MD5Anim>() { return "MD5Anim"; }
template <> inline unsigned int GetTypeID<resource::MD5Anim>() { return 1003; }

} // End of reflection
} // End of trillek

#endif
