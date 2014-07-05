#include "systems/physics.hpp"
#include "physics/collidable.hpp"
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
    // Do a static_pointer_cast to make sure we do have a Renderable component.
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
    static frame_tp last_tp;
    this->delta = timepoint - last_tp;
    last_tp = timepoint;

    // Set the rigid bodies linear velocity. Must be done each frame otherwise,
    // other forces will stop the linear velocity.
    for (auto force : this->forces) {
        auto body = this->bodies[force.first]->GetRigidBody();
        body->setLinearVelocity(force.second + body->getGravity());
    }

    if (this->dynamicsWorld) {
        dynamicsWorld->stepSimulation(delta.count() * 1.0E-9, 10);
    }

    // Set out transform updates.
    for (auto shape : this->bodies) {
        shape.second->UpdateTransform();
    }
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

void PhysicsSystem::SetForce(const unsigned int entity_id, const Force f) {
    this->forces[entity_id] = btVector3(f.x, f.y, f.z);
}

void PhysicsSystem::RemoveForce(const unsigned int entity_id) {
    if (this->forces.find(entity_id) != this->forces.end()) {
        this->forces.erase(entity_id);
    }
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
