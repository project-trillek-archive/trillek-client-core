#ifndef CAPSULE_SHAPE_HPP
#define CAPSULE_SHAPE_HPP

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

#include <memory>
#include <vector>

#include "type-id.hpp"

namespace trillek {

namespace resource {

class Mesh;

} // End of resource

class Transform;

namespace physics {

class Collidable {
public:
    Collidable() : motion_state(nullptr) { }
    ~Collidable();

    /**
     * \brief Initializes the component with the provided properties
     *
     * Valid properties include mesh (the mesh resource name)
     * \param[in] const std::vector<Property>& properties The creation properties for the component.
     * \return bool true if initialization finished with no errors.
     */
    bool Initialize(const std::vector<Property> &properties);

    /**
     * \brief Sets the shapes transform using the provided entity ID
     *
     * \param[in] unsigned int entity_id The ID of the entity to get the transform for.
     */
    void SetEntity(unsigned int entity_id);

    /**
     * \brief Initilizes the rigied body after the shape has been initialized.
     */
    bool InitializeRigidBody();

    /**
     * \brief Gets the shape's rigidbody.
     *
     * \return btRigidBody* The shape's rigidbody.
     */
    btRigidBody* GetRigidBody() const { return this->body.get(); };

    /**
     * \brief Updates the entity's transform with the current motion_state.
     */
    void UpdateTransform();

private:
    double radius; // Used for sphere and capsule shape collidable.
    double height; // Used for capsule shape collidable.
    btScalar mass; // For static objects mass must be 0.
    bool disable_deactivation; // Whether to disable automatic deactivation.

    std::shared_ptr<btTriangleMesh> mesh; // Used for mesh shape collidable.
    std::shared_ptr<resource::Mesh> mesh_file; // Used for mesh shape collidable.

    btMotionState* motion_state;
    std::shared_ptr<btCollisionShape> shape;
    std::shared_ptr<btRigidBody> body;

};

} // End of physics
} // End of trillek

#endif
