#include "systems/physics.hpp"
#include "physics/collidable.hpp"
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>
#include <Bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

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
        entity_bodies[shape->GetRigidBody()] = shape;
    }
}

void PhysicsSystem::HandleEvents(const frame_tp& timepoint) {
    static frame_tp last_tp;
    std::chrono::duration<double> delta = timepoint - last_tp;
    last_tp = timepoint;
    if (this->dynamicsWorld) {
        dynamicsWorld->stepSimulation(delta.count(), 10);
    }

    for (auto shape : this->entity_bodies) {
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

} // End of physics
} // End of trillek
