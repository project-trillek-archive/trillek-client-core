#include "systems/physics.hpp"
#include "physics/capsule-shape.hpp"

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

}

void PhysicsSystem::AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component) {
}

void PhysicsSystem::HandleEvents(const frame_tp& timepoint) {
    static frame_tp last_tp;
    std::chrono::duration<double> delta = timepoint - last_tp;
    last_tp = timepoint;
    if (this->dynamicsWorld) {
        dynamicsWorld->stepSimulation(delta.count(), 10);
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
