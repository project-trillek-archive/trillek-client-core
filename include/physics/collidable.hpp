#ifndef CAPSULE_SHAPE_HPP
#define CAPSULE_SHAPE_HPP

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

#include <memory>

#include "systems/component-factory.hpp"
#include "type-id.hpp"
#include "component.hpp"

namespace trillek {

namespace resource {

class Mesh;

} // End of resource

class Transform;

namespace physics {

class Collidable :
    public ComponentBase {
public:
    Collidable() : motion_state(nullptr) { }
    ~Collidable() {
        if (this->motion_state) {
            delete this->motion_state;
        }
    }

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

    /**
     * \brief Updates the current motion_state with the entity's transform.
     */
    void UpdateMotionState();

private:
    double radius; // Used for sphere and capsule shape collidable.
    double height; // Used for capsule shape collidable.
    btScalar mass; // For static objects mass must be 0.
    bool disable_deactivation; // Whether to disable automatic deactivation.

    std::unique_ptr<btTriangleMesh> mesh; // Used for mesh shape collidable.
    std::shared_ptr<resource::Mesh> mesh_file; // Used for mesh shape collidable.

    btMotionState* motion_state;
    std::unique_ptr<btCollisionShape> shape;
    std::unique_ptr<btRigidBody> body;

    std::shared_ptr<Transform> entity_transform;
};

} // End of physics
namespace reflection {

template <> inline const char* GetTypeName<physics::Collidable>() { return "collidable"; }
template <> inline const unsigned int GetTypeID<physics::Collidable>() { return 3000; }

} // End of reflection
} // End of trillek

#endif
