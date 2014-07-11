#include "resources/md5anim.hpp"

#include <fstream>
#include <memory>
#include <sstream>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "resources/md5mesh.hpp"

namespace trillek {
namespace resource {

void MD5Anim::Joint::ComputeW() {
    float t = 1.0f -
        (this->base_orientation[0] * this->base_orientation[0]) -
        (this->base_orientation[1] * this->base_orientation[1]) -
        (this->base_orientation[2] * this->base_orientation[2]);

    if (t < 0.0f) {
        this->base_orientation[3] = 0.0f;
    }
    else {
        this->base_orientation[3] = -sqrt(t);
    }
}

void MD5Anim::SkeletonJoint::ComputeW() {
    float t = 1.0f -
        (this->orientation[0] * this->orientation[0]) -
        (this->orientation[1] * this->orientation[1]) -
        (this->orientation[2] * this->orientation[2]);

    if (t < 0.0f) {
        this->orientation[3] = 0.0f;
    }
    else {
        this->orientation[3] = -sqrt(t);
    }
}

/**
* \brief Cleans an input string by removing certain grouping characters.
*
* These characters include ", ', (, and ).
* \param[in] std::string str The string to clean.
* \return The cleaned string
*/
extern std::string CleanString(std::string str);

bool MD5Anim::Initialize(const std::vector<Property> &properties) {
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "filename") {
            this->fname = p.Get<std::string>();
        }
    }

    if (Parse()) {
        for (size_t i = 0; i < this->frames.size(); ++i) {
            BuildFrameSkeleton(i);
        }
        return true;
    }

    return false;
}

bool MD5Anim::Parse() {
    std::ifstream f(this->fname, std::ios::in);

    if (!f.is_open()) {
        return false;
    }

    std::string line;
    unsigned int num_components;
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string identifier;

        ss >> identifier;
        if (identifier == "MD5Version") {
            int version;
            ss >> version;

            if (version != 10) {
                return false;
            }
        }
        else if (identifier == "numFrames") {
            int num_frames;
            ss >> num_frames;
            this->frames.reserve(num_frames);
        }
        else if (identifier == "numJoints") {
            int njoints;
            ss >> njoints;
            this->joints.reserve(njoints);
        }
        else if (identifier == "frameRate") {
            ss >> this->frame_rate;
        }
        else if (identifier == "numAnimatedComponents") {
            ss >> num_components;
        }
        else if (identifier == "hierarchy") {
            while (std::getline(f, line)) {
                if (line.find("\"") != std::string::npos) {
                    ss.str(CleanString(line));
                    std::unique_ptr<Joint> bone(new Joint());
                    ss >> bone->name;
                    ss >> bone->parent;
                    ss >> bone->flags;
                    ss >> bone->start_index;
                    this->joints.push_back(std::move(bone));
                }
                // Check if the line contained the closing brace. This is done after parsing
                // as the line might have the ending brace on it.
                if (line.find("}") != std::string::npos) {
                    break;
                }
            }
        }
        else if (identifier == "bounds") {
            while (std::getline(f, line)) {
                if ((line.find("(") != std::string::npos) && (line.find(")") != std::string::npos)) {
                    ss.str(CleanString(line));
                    std::unique_ptr<BoundingBox> bbox(new BoundingBox());
                    ss >> bbox->min[0]; ss >> bbox->min[1]; ss >> bbox->min[2];
                    ss >> bbox->max[0]; ss >> bbox->max[1]; ss >> bbox->max[2];
                    this->bounds.push_back(std::move(bbox));
                }
                // Check if the line contained the closing brace. This is done after parsing
                // as the line might have the ending brace on it.
                if (line.find("}") != std::string::npos) {
                    break;
                }
            }
        }
        else if (identifier == "baseframe") {
            size_t index = 0;
            while (std::getline(f, line)) {
                if ((line.find("(") != std::string::npos) && (line.find(")") != std::string::npos)) {
                    ss.str(CleanString(line));
                    auto& joint = this->joints[index];
                    // Check if the base frame block is malformed.
                    if (index > this->joints.size()) {
                        return false;
                    }
                    ss >> joint->base_position[0];
                    ss >> joint->base_position[1];
                    ss >> joint->base_position[2];
                    ss >> joint->base_orientation[0];
                    ss >> joint->base_orientation[1];
                    ss >> joint->base_orientation[2];
                }
                // Check if the line contained the closing brace. This is done after parsing
                // as the line might have the ending brace on it.
                if (line.find("}") != std::string::npos) {
                    break;
                }
                ++index;
            }
        }
        else if (identifier == "frame") {
            std::unique_ptr<Frame> frame(new Frame());
            ss >> frame->index;
            unsigned int number = 0;
            while (std::getline(f, line)) {
                ss.clear();
                ss.str(line);
                // Check if the line contained the closing brace. This is done after parsing
                // as the line might have the ending brace on it.
                if (line.find("}") != std::string::npos) {
                    break;
                }
                while (!ss.eof()) {
                    // Check if frame block is malformed.
                    if (number > num_components) {
                        return false;
                    }
                    float temp;
                    ss >> temp;
                    frame->parameters.push_back(temp);
                    ++number;
                }
            }
            this->frames.push_back(std::move(frame));
        }
    }

    return true;
}

void MD5Anim::BuildFrameSkeleton(size_t frame_index) {
    auto& frame = this->frames[frame_index];

    for (const auto& joint : this->joints) {
        unsigned int j = 0;

        // Start with the base frame position and orientation.
        SkeletonJoint skeleton_joint = { joint->parent, joint->base_position, joint->base_orientation };

        if (joint->flags & 1) { // Pos.x
            skeleton_joint.position.x = frame->parameters[joint->start_index + j++];
        }
        if (joint->flags & 2) { // Pos.y
            skeleton_joint.position.y = frame->parameters[joint->start_index + j++];
        }
        if (joint->flags & 4) { // Pos.x
            skeleton_joint.position.z = frame->parameters[joint->start_index + j++];
        }
        if (joint->flags & 8) { // Orient.x
            skeleton_joint.orientation.x = frame->parameters[joint->start_index + j++];
        }
        if (joint->flags & 16) { // Orient.y
            skeleton_joint.orientation.y = frame->parameters[joint->start_index + j++];
        }
        if (joint->flags & 32) { // Orient.z
            skeleton_joint.orientation.z = frame->parameters[joint->start_index + j++];
        }

        skeleton_joint.ComputeW();

        if (skeleton_joint.parent >= 0) {
            const auto& parent_joint = frame->skeleton.skeleton_joints[skeleton_joint.parent];
            glm::vec3 rotPos = parent_joint.orientation * skeleton_joint.position;

            skeleton_joint.position = parent_joint.position + rotPos;
            skeleton_joint.orientation = parent_joint.orientation * skeleton_joint.orientation;

            skeleton_joint.orientation = glm::normalize(skeleton_joint.orientation);
        }

        frame->skeleton.skeleton_joints.push_back(std::move(skeleton_joint));

        glm::mat4 bone_translate = glm::translate(glm::mat4(1.0f), skeleton_joint.position);
        glm::mat4 bone_rotate = glm::toMat4(skeleton_joint.orientation);
        glm::mat4 bone_matrix = bone_translate * bone_rotate;

        frame->skeleton.bone_matricies.push_back(bone_matrix);
    }
}

bool MD5Anim::CheckMesh(std::shared_ptr<Mesh> mesh_file) {
    std::shared_ptr<MD5Mesh> mesh = std::static_pointer_cast<MD5Mesh>(mesh_file);
    if (mesh) {
        // Make sure number of joints matches.
        if (this->joints.size() != mesh->joints.size()) {
            return false;
        }

        for (size_t i = 0; i < this->joints.size(); ++i) {
            // Make sure joint names and parents match up.
            if ((this->joints[i]->name != mesh->joints[i]->name) || 
                (this->joints[i]->parent != mesh->joints[i]->parent)) {
                return false;
            }
            this->joints[i]->bind_pose_inverse = mesh->joints[i]->bind_pose_inverse;
        }
        return true;
    }
    return false;
}

MD5Anim::FrameSkeleton MD5Anim::InterpolateSkeletons(size_t frame_index_start,
    size_t frame_index_end, float delta) {
    const auto& skeleton0 = this->frames[frame_index_start]->skeleton;
    const auto& skeleton1 = this->frames[frame_index_end]->skeleton;
    FrameSkeleton final_skeleton;

    size_t num_joints = this->joints.size();

    final_skeleton.skeleton_joints.insert(final_skeleton.skeleton_joints.begin(),
        num_joints, SkeletonJoint());
    final_skeleton.bone_matricies.insert(final_skeleton.bone_matricies.begin(),
        num_joints, glm::mat4(1.0f));
    
    for (size_t i = 0; i < num_joints; ++i) {
        SkeletonJoint& finalJoint = final_skeleton.skeleton_joints[i];
        glm::mat4& finalMatrix = final_skeleton.bone_matricies[i];

        const SkeletonJoint& joint0 = skeleton0.skeleton_joints[i];
        const SkeletonJoint& joint1 = skeleton1.skeleton_joints[i];

        finalJoint.parent = joint0.parent;

        finalJoint.position = glm::lerp(joint0.position, joint1.position, delta);
        finalJoint.orientation = glm::mix(joint0.orientation, joint1.orientation, delta);

        // Build the bone matrix for GPU skinning.
        finalMatrix = (glm::translate(glm::mat4(1.0f), finalJoint.position) *
            glm::toMat4(finalJoint.orientation)) * this->joints[i]->bind_pose_inverse;
    }

    return std::move(final_skeleton);
}

} // End of resource
} // End of trillek
