#include "trillek-game.hpp"
#include "components/shared-component.hpp"
#include "components/system-component.hpp"
#include "components/system-component-value.hpp"
#include "physics/collidable.hpp"
#include "systems/transform-system.hpp"
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include "logging.hpp"

namespace trillek { namespace physics {

using namespace component;

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

void PhysicsSystem::HandleEvents(frame_tp timepoint) {
    // publish the forces of the current frame immediately without making a copy of the list
    for (auto& v : this->velocities.Poll()) {
        Update<Component::Velocity>(std::move(v.first), std::move(v.second));
    }
    // commit velocity updates
    Commit<Component::Velocity>(timepoint);

    static frame_tp last_tp;
    this->delta = timepoint - last_tp;
    last_tp = timepoint;

    // Set the rigid bodies linear velocity. Must be done each frame otherwise,
    // other forces will stop the linear velocity.
    // We use the published list

    // First moving entities that have no combined velocity
    OnTrue(GetLastPositiveBitMap<Component::Velocity>()
                                        & ~Bitmap<Component::ReferenceFrame>(),
        [](id_t entity_id) {
            // first inject velocity of entities that have no reference frame
            auto& body = *Get<Component::Collidable>(entity_id).GetRigidBody();
            const auto& v = Get<Component::Velocity>(entity_id);

            body.setLinearVelocity(v.linear + body.getGravity());
            body.setAngularVelocity(v.angular);
        }
    );
    // Second moving entities with a combined velocity
    OnTrue(GetLastPositiveBitMap<Component::Velocity>()
                                        & Bitmap<Component::ReferenceFrame>(),
        [](id_t entity_id) {
            // combine velocity
            auto reference_id = Get<Component::ReferenceFrame>(entity_id);
            const auto& v = Get<Component::Velocity>(entity_id);
            const auto& ref_v = Get<Component::Velocity>(reference_id);
            auto& body = *Get<Component::Collidable>(reference_id).GetRigidBody();
            auto& transform = body.getCenterOfMassTransform();
            auto combined_l = transform * v.linear;
            combined_l += ref_v.linear;
            body.setLinearVelocity(combined_l + body.getGravity());
            auto combined_a = transform * v.angular;
            combined_a += ref_v.angular;
            body.setAngularVelocity(combined_a);
        }
    );

    // Third, entities with reference frame that have moved
    OnTrue(GetLastPositiveBitMap<Component::Velocity>()
                                        & Bitmap<Component::IsReferenceFrame>(),
        [&](id_t entity_id) {
            // todo
        }
    );


    dynamicsWorld->stepSimulation(delta * 1.0E-9, 10);
    // Set out transform updates.
    auto& bodymap = TrillekGame::GetSystemComponent().Map<Component::Collidable>();
    for (auto& shape : bodymap) {
        btTransform transform;
        shape.second->Get<Collidable>().GetRigidBody()->getMotionState()->getWorldTransform(transform);

        auto pos = transform.getOrigin();
        auto rot = transform.getRotation();
        Transform_type entity_transform(Get<Component::Transform>(shape.first));
        entity_transform.SetTranslation(glm::vec3(pos.x(), pos.y(), pos.z()));
        entity_transform.SetOrientation(glm::quat(rot.w(), rot.x(), rot.y(), rot.z()));
        Update<Component::Transform>(shape.first, std::move(entity_transform));
    }
    // Publish the new updated transforms map
    Commit<Component::Transform>(timepoint);
}

void PhysicsSystem::AddDynamicComponent(const unsigned int entity_id, std::shared_ptr<Container> component) {
    if (component->Is<Collidable>()) {
        AddBodyToWorld(component->Get<Collidable>().GetRigidBody());
    }
}

void PhysicsSystem::AddBodyToWorld(btRigidBody* body) {
    this->dynamicsWorld->addRigidBody(body);
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

void PhysicsSystem::SetGravity(const unsigned int entity_id, const btVector3* f) {
    auto& system = TrillekGame::GetSystemComponent();
    if (system.Has<Component::Collidable>(entity_id)) {
        if (f != nullptr) {
            system.Get<Component::Collidable>(entity_id).GetRigidBody()
                                                            ->setGravity(*f);
        }
        else {
            system.Get<Component::Collidable>(entity_id).GetRigidBody()
                            ->setGravity(this->dynamicsWorld->getGravity());
        }
    }
}

} // End of physics
} // End of trillek
