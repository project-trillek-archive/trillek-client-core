#include "transform.hpp"
#include "systems/transform-system.hpp"

namespace trillek {

Transform::Transform(unsigned int entity_id) :
    orientation(glm::quat(1, 0, 0, 0)), scale(1.0f), entity_id(entity_id) {
}

void Transform::Translate(const glm::vec3 amount) {
    this->translation += amount;
}

void Transform::Rotate(const glm::vec3 amount) {
    this->rotation += amount;

    glm::quat change(this->rotation);
    this->orientation = glm::normalize(change * this->orientation);
}

void Transform::OrientedTranslate(const glm::vec3 amount) {
    this->translation += this->orientation * amount;
}

void Transform::OrientedRotate(const glm::vec3 amount) {
    this->rotation += amount;

    glm::quat qX = glm::angleAxis(amount.x, this->orientation * RIGHT_VECTOR);
    glm::quat qY = glm::angleAxis(amount.y, this->orientation * UP_VECTOR);
    glm::quat qZ = glm::angleAxis(amount.z, this->orientation * glm::vec3(0.0, 0.0, 1.0)); // Z axis not Forward
    glm::quat change = qX * qY * qZ;

    this->orientation = glm::normalize(change * this->orientation);
}

void Transform::Scale(const glm::vec3 amount) {
    this->scale *= amount;
}

void Transform::SetTranslation(const glm::vec3 new_translation) {
    this->translation = new_translation;
}

void Transform::SetRotation(const glm::vec3 new_rotation) {
    this->orientation = glm::normalize(glm::quat(new_rotation));
    this->rotation.x = atan2(2.0f * (this->orientation.y * this->orientation.z +
        this->orientation.w * this->orientation.x),
        this->orientation.w * this->orientation.w - this->orientation.x *
        this->orientation.x - this->orientation.y * this->orientation.y +
        this->orientation.z * this->orientation.z);
    this->rotation.y = glm::asin(-2.0f * (this->orientation.x * this->orientation.z - this->orientation.w * this->orientation.y));
    this->rotation.z = atan2(2.0f * (this->orientation.x * this->orientation.y +
        this->orientation.w * this->orientation.z),
        this->orientation.w * this->orientation.w + this->orientation.x *
        this->orientation.x - this->orientation.y * this->orientation.y -
        this->orientation.z * this->orientation.z);
}

void Transform::SetOrientation(const glm::quat new_orientation) {
    this->orientation = new_orientation;
    this->rotation.x = atan2(2.0f * (this->orientation.y * this->orientation.z +
        this->orientation.w * this->orientation.x),
        this->orientation.w * this->orientation.w - this->orientation.x *
        this->orientation.x - this->orientation.y * this->orientation.y +
        this->orientation.z * this->orientation.z);
    this->rotation.y = glm::asin(-2.0f * (this->orientation.x * this->orientation.z - this->orientation.w * this->orientation.y));
    this->rotation.z = atan2(2.0f * (this->orientation.x * this->orientation.y +
        this->orientation.w * this->orientation.z),
        this->orientation.w * this->orientation.w + this->orientation.x *
        this->orientation.x - this->orientation.y * this->orientation.y -
        this->orientation.z * this->orientation.z);
}

void Transform::SetScale(const glm::vec3 new_scale) {
    this->scale = new_scale;
}

glm::vec3 Transform::GetTranslation() const {
    return this->translation;
}

glm::vec3 Transform::GetRotation() const {
    return this->rotation;
}

glm::quat Transform::GetOrientation() const {
    return this->orientation;
}

glm::vec3 Transform::GetScale() const {
    return this->scale;
}
} // End of trillek
