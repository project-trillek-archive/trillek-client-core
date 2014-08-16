#include "physics/collidable.hpp"
#include "transform.hpp"
#include "systems/transform-system.hpp"
#include "systems/resource-system.hpp"
#include "resources/mesh.hpp"

#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>

namespace trillek {
namespace physics {

std::unique_ptr<btTriangleMesh> GenerateTriangleMesh(std::shared_ptr<resource::Mesh> mesh_file) {
    auto mesh = std::unique_ptr<btTriangleMesh>(new btTriangleMesh());
    if (!mesh_file) {
        return nullptr;
    }
    for (size_t mesh_i = 0; mesh_i < mesh_file->GetMeshGroupCount(); ++mesh_i) {
        const auto& mesh_group = mesh_file->GetMeshGroup(mesh_i);
        const auto& temp_lock = mesh_group.lock();
        for (size_t face_i = 0; face_i < temp_lock->indicies.size(); ++face_i) {
            const resource::VertexData& v1 = temp_lock->verts[temp_lock->indicies[face_i]];
            const resource::VertexData& v2 = temp_lock->verts[temp_lock->indicies[++face_i]];
            const resource::VertexData& v3 = temp_lock->verts[temp_lock->indicies[++face_i]];
            mesh->addTriangle(
                btVector3(v1.position.x, v1.position.y, v1.position.z),
                btVector3(v2.position.x, v2.position.y, v2.position.z),
                btVector3(v3.position.x, v3.position.y, v3.position.z), true);
        }
    }

    return std::move(mesh);
}

bool Collidable::Initialize(const std::vector<Property> &properties) {
    std::string shape = "sphere";
    std::string mesh_name;
    this->radius = 1.0;
    this->height = 1.0;
    this->mass = 1.0;
    unsigned int entity_id;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "radius") {
            this->radius = p.Get<double>();
        }
        else if (name == "disable_deactivation") {
            this->disable_deactivation = p.Get<bool>();
        }
        else if (name == "mass") {
            this->mass = p.Get<double>();
        }
        else if (name == "height") {
            this->height = p.Get<double>();
        }
        else if (name == "shape") {
            shape = p.Get<std::string>();
        }
        else if (name == "mesh") {
            mesh_name = p.Get<std::string>();
        }
        else if (name == "entity_id") {
            entity_id = p.Get<unsigned int>();
        }
    }

    SetEntity(entity_id);
    if (!this->entity_transform) {
        return false;
    }

    if (shape == "capsule") {
        this->shape = std::move(std::unique_ptr<btCollisionShape>(new btCapsuleShape(this->radius, this->height)));
    }
    else if (shape == "sphere") {
        this->shape = std::move(std::unique_ptr<btCollisionShape>(new btSphereShape(this->radius)));
    }
    else if (shape == "static_mesh") {
        this->mesh_file = resource::ResourceMap::Get<resource::Mesh>(mesh_name);
        this->mesh = GenerateTriangleMesh(this->mesh_file);
        if (!this->mesh) {
            return false;
        }

        auto scale = this->entity_transform->GetScale();
        auto mesh_shape = std::unique_ptr<btScaledBvhTriangleMeshShape>(
            new btScaledBvhTriangleMeshShape(
            new btBvhTriangleMeshShape(this->mesh.get(), true), btVector3(scale.x, scale.y, scale.z)));
        this->shape = std::move(mesh_shape);

        // Static BvhTriangleMehes must have a mass of 0.
        this->mass = 0;
    }
    else if (shape == "dynamic_mesh") {
        this->mesh_file = resource::ResourceMap::Get<resource::Mesh>(mesh_name);
        this->mesh = GenerateTriangleMesh(this->mesh_file);
        if (!this->mesh) {
            return false;
        }

        auto scale = this->entity_transform->GetScale();

        auto mesh_shape = std::unique_ptr<btGImpactMeshShape>(new btGImpactMeshShape(this->mesh.get()));
        mesh_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
        mesh_shape->updateBound();
        this->shape = std::move(mesh_shape);
    }

    if (!this->shape) {
        return false;
    }
    return InitializeRigidBody();
}

void Collidable::SetEntity(unsigned int entity_id) {
    this->entity_transform = TransformMap::GetTransform(entity_id);
    if(!this->entity_transform) {
        return;
    }
    auto pos = this->entity_transform->GetTranslation();
    auto orientation = this->entity_transform->GetOrientation();
    this->motion_state = new btDefaultMotionState(btTransform(
        btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w), btVector3(pos.x, pos.y, pos.z)));
}

bool Collidable::InitializeRigidBody() {
    btVector3 fallInertia(0, 0, 0);
    if (this->mass) {
        this->shape->calculateLocalInertia(mass, fallInertia);
    }
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(this->mass,
        this->motion_state, this->shape.get(), fallInertia);
    this->body = std::move(std::unique_ptr<btRigidBody>(new btRigidBody(fallRigidBodyCI)));

    if (!this->body) {
        return false;
    }

    // Check if we want to disable automatic deactivation for this body.
    if (this->disable_deactivation) {
        this->body->forceActivationState(DISABLE_DEACTIVATION);
    }

    // Prevent objects from rotating from physics system.
    this->body->setAngularFactor(btVector3(0, 0, 0));

    return true;
}

void Collidable::UpdateTransform() {
    if (!this->motion_state && !this->entity_transform) {
        return;
    }
    btTransform transform;
    this->motion_state->getWorldTransform(transform);

    auto pos = transform.getOrigin();
    auto rot = transform.getRotation();
    this->entity_transform->SetTranslation(glm::vec3(pos.x(), pos.y(), pos.z()));
    this->entity_transform->SetOrientation(glm::quat(rot.w(), rot.x(), rot.y(), rot.z()));
    this->entity_transform->MarkAsModified();
}

void Collidable::UpdateMotionState() {
    if (!this->entity_transform) {
        return;
    }
    auto pos = this->entity_transform->GetTranslation();
    auto orientation = this->entity_transform->GetOrientation();
    btTransform transform;
    this->motion_state->getWorldTransform(transform);
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    transform.setRotation(btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w));
    this->motion_state->setWorldTransform(transform);
}

/* Removed since we don't have to push transforms into the simulation but to apply forces
void Collidable::Notify(const unsigned int entity_id, const Transform* transforum) {
    auto pos = this->entity_transform->GetTranslation();
    auto orientation = this->entity_transform->GetOrientation();
    this->body->setWorldTransform(btTransform(
        btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w), btVector3(pos.x, pos.y, pos.z)));
}
*/
} // End of physics
} // End of trillek
