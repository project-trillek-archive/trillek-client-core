#ifndef TRANSFORM_HPP_INCLUDED
#define TRANSFORM_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

namespace trillek {

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
     */
    void Translate(const glm::vec3 amount);

    /**
     * \brief Rotates by the provided amount relative to the current rotation.
     *
     * The arguments are in the form of (rotation about x, rotation about y, rotation about z).
     * The orientation is also updated by computing the axis-angle.
     * \param[in] const glm::vec3 amount The amount to rotate by.
     */
    void Rotate(const glm::vec3 amount);

    /**
     * \brief Translates by the provided amount relative to the current translation and orientation.
     *
     * \param[in] const glm::vec3 amount The amount to translate by.
     */
    void OrientedTranslate(const glm::vec3 amount);

    /**
     * \brief Rotates by the provided amount relative to the current rotation and orientation.
     *
     * The arguments are in the form of (rotation about x, rotation about y, rotation about z).
     * The orientation is also updated by computing the axis-angle.
     * \param[in] const glm::vec3 amount The amount to rotate by.
     */
    void OrientedRotate(const glm::vec3 amount);

    /**
     * \brief Scales by the provided amount relative to the current scale.
     *
     * The current scale is multiplied by the given amount.
     * \param[in] const glm::vec3 amount The amount to scale by.
     */
    void Scale(const glm::vec3 amount);

    /**
     * \brief Sets the translation.
     *
     * \param[in] const glm::vec3 new_translation The new translation.
     */
    void SetTranslation(const glm::vec3 new_translation);

    /**
     * \brief Sets the rotation.
     *
     * The arguments are in the form of (rotation about x, rotation about y, rotation about z).
     * The orientation is also set by computing the axis-angle.
     * \param[in] const glm::vec3 new_rotation The new rotation.
     */
    void SetRotation(const glm::vec3 new_rotation);

    /**
     * \brief Sets the rotation.
     *
     * \param[in] const glm::quat new_orientation The new orientation.
     */
    void SetOrientation(const glm::quat new_orientation);

    /**
     * \brief Sets the scale.
     *
     * \param[in] const glm::vec3 new_scale The new scale.
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

    /** \brief Mark the transform as modified during the current frame
     *
     */
    void MarkAsModified();

    unsigned int GetEntityID() const {
        return this->entity_id;
    }
private:
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::quat orientation;
    unsigned int entity_id;
};

} // End of trillek

#endif
