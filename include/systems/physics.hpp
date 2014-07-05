#ifndef PHYSICS_HPP_INCLUDED
#define PHYSICS_HPP_INCLUDED

#include <bullet/btBulletDynamicsCommon.h>

#include <memory>
#include <map>

#include "trillek-scheduler.hpp"
#include "systems/system-base.hpp"

namespace trillek {
namespace physics {

class Collidable;

struct Force {
    double x, y, z;
};

class PhysicsSystem : public SystemBase {
public:
    PhysicsSystem();
    ~PhysicsSystem();
    /**
     * \brief Starts the Simple Physics system.
     *
     * \return bool Returns false on startup failure.
     */
    void Start();

    void ThreadInit() override { }

    /**
     * \brief Causes an update in the system based on the change in time.
     *
     * Updates the state of the system based off how much time has elapsed since the last update.
     * \return void
     */
    void RunBatch() const override {};

    /**
     * \brief Adds a Shape component to the system.
     *
     * A dynamic_pointer_case is applied to the component shared_ptr to cast it to
     * a Shape component. If the cast results in a nullptr the method returns
     * without adding the Shape component.
     * \param const unsigned int entity_id The entity ID the compoennt belongs to.
     * \param std::shared_ptr<ComponentBase> component The component to add.
     * \return void
     */
    void AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component);


    /** \brief Handle incoming events to update data
     *
     * This function is called once every frame. It is the only
     * function that can write data. This function is in the critical
     * path, job done here must be simple.
     *
     * If event handling need some batch processing, a task list must be
     * prepared and stored temporarily to be retrieved by RunBatch().
     *
     */
    void HandleEvents(const frame_tp& timepoint) override;

    /** \brief Save the data and terminate the system
     *
     * This function is called when the program is closing
     *
     */
    void Terminate() override;

    /** \brief Set a rigid body's current linear force.
     *
     * \param const unsigned int entity_id The entity ID of the rigid body.
     * \param Force f The rigid body's new force.
     * \return void
     */
    void SetForce(const unsigned int entity_id, const Force f);

    /** \brief Remove a rigid body's current linear force.
    *
    * \param const unsigned int entity_id The entity ID of the rigid body.
     * \return void
    */
    void RemoveForce(const unsigned int entity_id);

    /** \brief Set a rigid body's gravity.
    *
    * \param const unsigned int entity_id The entity ID of the rigid body.
    * \param const Force* f The rigid body's new gravity (world gravity if nullptr).
    * \return void
    */
    void SetGravity(const unsigned int entity_id, const Force* f = nullptr);
private:
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;

    std::map<unsigned int, std::shared_ptr<Collidable>> bodies;

    std::map<unsigned int, btVector3> forces;

    btCollisionShape* groundShape;
    btDefaultMotionState* groundMotionState;
    btRigidBody* groundRigidBody;

    frame_unit delta; // The time since the last HandleEvents was called.
};

} // End of physcics
} // End of trillek
#endif
