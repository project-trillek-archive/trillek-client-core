#include "resources/transform.hpp"

namespace trillek {
namespace transform {

Transform::Transform(unsigned int entity_id) :
    orientation(glm::quat(1, 0, 0, 0)), scale(1.0f), entity_id(entity_id) {
}

void Transform::Translate(const glm::vec3 amount) {
    this->translation += amount;
}

void Transform::Rotate(const glm::vec3 amount) {
    this->rotation += amount;

    glm::quat qX = glm::angleAxis(amount.x, RIGHT_VECTOR);
    glm::quat qY = glm::angleAxis(amount.y, UP_VECTOR);
    glm::quat qZ = glm::angleAxis(amount.z, FORWARD_VECTOR);
    glm::quat change = qX * qY * qZ;

    this->orientation = glm::normalize(change * this->orientation);
}

void Transform::Scale(const glm::vec3 amount) {
    this->scale += amount;
}

void Transform::SetTranslation(const glm::vec3 new_translation) {
    this->translation = new_translation;
}

void Transform::SetRotation(const glm::vec3 new_rotaiton) {
    this->rotation = new_rotaiton;

    glm::quat qX = glm::angleAxis(new_rotaiton.x, RIGHT_VECTOR);
    glm::quat qY = glm::angleAxis(new_rotaiton.y, UP_VECTOR);
    glm::quat qZ = glm::angleAxis(new_rotaiton.z, FORWARD_VECTOR);
    glm::quat change = qX * qY * qZ;

    this->orientation = glm::normalize(change);
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

} // End of transform
} // End of trillek
