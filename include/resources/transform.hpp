#ifndef TRANSFORM_HPP_INCLUDED
#define TRANSFORM_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

namespace trillek {
namespace transform {

static glm::vec3 FORWARD_VECTOR(0.0f, 0.0f, -1.0f);
static glm::vec3 UP_VECTOR(0.0f, 1.0f, 0.0f);
static glm::vec3 RIGHT_VECTOR(1.0f, 0.0f, 0.0f);

class Transform {
public:
    Transform(unsigned int entity_id);
    /**
    * \brief Translates by the provided amount relative to the current translation.
    *
    * \param[in] const glm::vec3 amount The amount to translate by.
    * \return
    */
    void Translate(const glm::vec3 amount);

    /**
    * \brief Rotates by the provided amount relative to the current rotation.
    *
    * The arguments is in the form of (rotation about x, rotation about y, rotation about z).
    * The orientation is also updated by computing the axis-angle.
    * \param[in] const glm::vec3 amount The amount ot rotate by.
    * \return
    */
    void Rotate(const glm::vec3 amount);

    /**
    * \brief Scales by the provided amount relative to the current scale.
    *
    * The current scale is multiplied by the given amount.
    * \param[in] const glm::vec3 amount The amount to scale by.
    * \return
    */
    void Scale(const glm::vec3 amount);

    /**
    * \brief Sets the translation.
    *
    * \param[in] const glm::vec3 new_translation The new translation.
    * \return
    */
    void SetTranslation(const glm::vec3 new_translation);

    /**
    * \brief Sets the rotation.
    *
    * The arguments is in the form of (rotation about x, rotation about y, rotation about z).
    * The orientation is also set by computing the axis-angle.
    * \param[in] const glm::vec3 new_rotaiton The new translation.
    * \return
    */
    void SetRotation(const glm::vec3 new_rotaiton);

    /**
    * \brief Sets the scale.
    *
    * \param[in] const glm::vec3 new_scale The new scale.
    * \return
    */
    void SetScale(const glm::vec3 new_scale);

    /**
    * \brief Translates by the provided amount relative to the current translation
    *
    * \return glm::vec3 The current transform.
    */
    glm::vec3 GetTranslation() const;

    /**
    * \brief Returns the current rotation, not orientation.
    *
    * \return glm::vec3 The current rotation.
    */
    glm::vec3 GetRotation() const;

    /**
    * \brief Returns the current orientation, not rotation.
    *
    * \return glm::quat The current orientation.
    */
    glm::quat GetOrientation() const;

    /**
    * \brief Returns the current scale.
    *
    * \return glm::vec3 The current scale.
    */
    glm::vec3 GetScale() const;
private:
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::quat orientation;
    unsigned int entity_id;
};

} // End of resource
} // End of trillek

#endif
