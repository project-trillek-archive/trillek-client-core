#include "systems/physics.hpp"
#include "physics/collidable.hpp"
#include "systems/transform-system.hpp"
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

namespace trillek {
namespace physics {

PhysicsSystem::PhysicsSystem() { }
PhysicsSystem::~PhysicsSystem() { }

void PhysicsSystem::Start() {
    this->collisionConfiguration = new btDefaultCollisionConfiguration();
    this->dispatcher = new btCollisionDispatcher(this->collisionConfiguration);
    this->broadphase = new btDbvtBroadphase();
    this->solver = new btSequentialImpulseConstraintSolver();
    this->dynamicsWorld = new btDiscreteDynamicsWorld(this->dispatcher, this->broadphase, this->solver, this->collisionConfiguration);
    this->dynamicsWorld->setGravity(btVector3(0, -10, 0));

    // Register the collision dispatcher with the GImpact algorithm for dynamic meshes.
    btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher *>(this->dynamicsWorld->getDispatcher());
    btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
}

void PhysicsSystem::AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component) {
    std::shared_ptr<Collidable> shape = std::dynamic_pointer_cast<Collidable>(component);
    if (!shape) {
        return;
    }

    if (this->dynamicsWorld) {
        this->dynamicsWorld->addRigidBody(shape->GetRigidBody());
        this->bodies[entity_id] = shape;
    }
}

void PhysicsSystem::HandleEvents(const frame_tp& timepoint) {
    // Updated the motions state of all bodies in case it was changed outside physics (e.g scripting).
    for (auto& shape : this->bodies) {
        shape.second->UpdateMotionState();
    }

    // Remove access to old updated transforms
    TransformMap::GetAsyncUpdatedTransforms().Unpublish(timepoint);
    // Remove access to forces
    this->async_forces.Unpublish(timepoint);
    // Remove access to torques
    this->async_torques.Unpublish(timepoint);
    // publish the forces of the current frame immediately without making a copy of the list
    this->async_forces.Publish(std::make_shared<const std::map<id_t, btVector3>>(this->forces.Poll()));
    // publish the torques of the current frame
    this->async_torques.Publish(std::make_shared<const std::map<id_t, btVector3>>(this->torques.Poll()));

    static frame_tp last_tp;
    this->delta = timepoint - last_tp;
    last_tp = timepoint;

    // Set the rigid bodies linear velocity. Must be done each frame otherwise,
    // other forces will stop the linear velocity.
    // We use the published list
    for (auto& force : *this->async_forces.GetFuture(timepoint).get()) {
        auto body = this->bodies[force.first]->GetRigidBody();
        body->setLinearVelocity(force.second + body->getGravity());
    }
    // Set the rigid bodies angular velocity. Must be done each frame otherwise,
    // other forces will stop the angular velocity.
    for (auto& torque : *this->async_torques.GetFuture(timepoint).get()) {
        auto body = this->bodies[torque.first]->GetRigidBody();
        body->setAngularVelocity(torque.second);
    }
    if (this->dynamicsWorld) {
        this->dynamicsWorld->stepSimulation(this->delta.count() * 1.0E-9, 10);
    }
    // Set out transform updates.
    for (auto& shape : this->bodies) {
        shape.second->UpdateTransform();
    }
    // Publish the new updated transforms map
    auto ntm = std::make_shared<std::map<id_t,const Transform*>>(TransformMap::GetUpdatedTransforms().Poll());
    TransformMap::GetAsyncUpdatedTransforms().Publish(std::move(ntm));
}

void PhysicsSystem::Terminate() {
    if (this->dynamicsWorld != nullptr) {
        delete this->dynamicsWorld;
    }
    if (this->solver != nullptr) {
        delete this->solver;
    }
    if (this->collisionConfiguration != nullptr) {
        delete this->collisionConfiguration;
    }
    if (this->dispatcher != nullptr) {
        delete this->dispatcher;
    }
    if (this->broadphase != nullptr) {
        delete this->broadphase;
    }
}

void PhysicsSystem::SetForce(unsigned int entity_id, const Force f) const {
    this->forces.Insert(entity_id, btVector3(f.x, f.y, f.z));
}

void PhysicsSystem::RemoveForce(const unsigned int entity_id) const {
    this->forces.Erase(entity_id);
}

void PhysicsSystem::SetTorque(unsigned int entity_id, const Torque t) const {
    this->torques.Insert(entity_id, btVector3(t.x, t.y, t.z));
}

void PhysicsSystem::RemoveTorque(const unsigned int entity_id) const {
    this->torques.Erase(entity_id);
}

void PhysicsSystem::SetGravity(const unsigned int entity_id, const Force* f) {
    if (this->bodies.find(entity_id) != this->bodies.end()) {
        if (f != nullptr) {
            this->bodies.at(entity_id)->GetRigidBody()->setGravity(btVector3(f->x, f->y, f->z));
        }
        else {
            this->bodies.at(entity_id)->GetRigidBody()->setGravity(this->dynamicsWorld->getGravity());
        }
    }
}

} // End of physics
} // End of trillek
