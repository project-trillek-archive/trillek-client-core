#ifndef ANIMATION_HPP_INCLUDED
#define ANIMATION_HPP_INCLUDED

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace trillek {
namespace resource {

class MD5Anim;

} // End of resource

namespace graphics {

class Animation final {
public:
    Animation() : animation_time(0.0f) { }

    /**
    * \brief Updates the current animation based on a change in time.
    *
    * \param[in] float delta The change in time
    * \return void
    */
    void UpdateAnimation(const float delta);

    /**
    * \brief Sets the animation file for this animation.
    *
    * This does a run for the first set of animation from frame 0 to 1 with time 0.
    * \param[in] std::shared_ptr<resource::MD5Anim> file The animation file.
    * \return void
    */
    void SetAnimationFile(std::shared_ptr<resource::MD5Anim> file);

    friend class RenderSystem;
private:
    std::vector<glm::mat4> animation_matricies;

    std::shared_ptr<resource::MD5Anim> animation_file;

    size_t current_frame_index;
    size_t frame_count;

    float animation_time;
    float frame_duration;
    float frame_rate;
};

} // End of graphics
} // End of trillek

#endif
