#include "systems/physics.hpp"
#include "physics/collidable.hpp"

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

    groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    groundMotionState =
        new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -10, 0)));
    btRigidBody::btRigidBodyConstructionInfo
        groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    this->dynamicsWorld->addRigidBody(groundRigidBody);
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
    if (this->groundRigidBody != nullptr) {
        this->dynamicsWorld->removeRigidBody(groundRigidBody);
        delete this->groundRigidBody;
    }
    if (this->groundMotionState != nullptr) {
        delete this->groundMotionState;
    }
    if (this->groundShape != nullptr) {
        delete this->groundShape;
    }
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
