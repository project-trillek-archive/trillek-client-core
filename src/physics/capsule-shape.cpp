#include "physics/capsule-shape.hpp"
#include "transform.hpp"
#include "systems/transform-system.hpp"

namespace trillek {
namespace physics {

bool CapsuleShape::Initialize(const std::vector<Property> &properties) {
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "radius") {
            this->radius = p.Get<double>();
        }
        else if (name == "height") {
            this->height = p.Get<double>();
        }
    }
    return true;
}

void CapsuleShape::SetEntity(unsigned int entity_id) {
    this->entity_transform = TransformMap::GetTransform(entity_id);
    auto pos = this->entity_transform->GetTranslation();
    auto orientation = this->entity_transform->GetOrientation();
    this->motion_state = new btDefaultMotionState(btTransform(
        btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w), btVector3(pos.x, pos.y, pos.z)));
}

void CapsuleShape::InitializeRigidBody() {
	btCapsuleShape* shape = new btCapsuleShape(this->radius, this->height);
	btScalar mass = 1;
	btVector3 fallInertia(0,0,0);
    shape->calculateLocalInertia(mass, fallInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, motion_state, shape, fallInertia);
    btRigidBody* body = new btRigidBody(fallRigidBodyCI);

    this->shape = std::move(std::unique_ptr<btCollisionShape>(shape));
    this->body = std::move(std::unique_ptr<btRigidBody>(body));
}

void CapsuleShape::UpdateTransform() {
    btTransform transform;
    this->motion_state->getWorldTransform(transform);

    auto pos = transform.getOrigin();
    auto rot = transform.getRotation();
    this->entity_transform->SetTranslation(glm::vec3(pos.x(), pos.y(), pos.z()));
    this->entity_transform->SetOrientation(glm::quat(rot.w(), rot.x(), rot.y(), rot.z()));
}

} // End of physics
} // End of trillek
