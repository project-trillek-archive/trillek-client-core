#ifndef TRANSFORMSYSTEM_HPP_INCLUDED
#define TRANSFORMSYSTEM_HPP_INCLUDED

#include <memory>
#include <map>
#include <mutex>

#include "util/json-parser.hpp"
#include "systems/async-data.hpp"
#include "atomic-map.hpp"

namespace trillek {

class Transform;
namespace physics {
class PhysicsSystem;
}

// Stores a mapping of entity ID to transform that can
// be accessed via static methods anywhere.
class TransformMap : public util::Parser {
private:
    TransformMap() : Parser("transforms") { }
    TransformMap(const TransformMap& right) : Parser("transforms") {
        instance = right.instance;
    }
    TransformMap& operator=(const TransformMap& right) {
        if (this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<TransformMap> instance;
public:
    static std::shared_ptr<TransformMap> GetInstance() {
        std::call_once(TransformMap::only_one,
            [ ] () {
            TransformMap::instance.reset(new TransformMap());
        }
        );

        return TransformMap::instance;
    }
    ~TransformMap() { }

    /**
    * \brief Gets an entity's transform.
    *
    * \param[in] const unsigned int entity_id ID of the entity to get the transform for.
    * \return std::shared_ptr<resource::Transform> The entity's transofmr (nullptr if no transform found).
    */
    static std::shared_ptr<Transform> AddTransform(const unsigned int entity_id);

    /**
    * \brief Gets an entity's transform.
    *
    * \param[in] const unsigned int entity_id ID of the entity to get the transform for.
    * \return std::shared_ptr<resource::Transform> The entity's transofmr (nullptr if no transform found).
    */
    static std::shared_ptr<Transform> GetTransform(const unsigned int entity_id);

    /**
    * \brief Gets an entity's transform.
    *
    * \param[in] const unsigned int entity_id ID of the entity to get the transform for.
    * \return void
    */
    static void RemoveTransform(const unsigned int entity_id);

    static AsyncData<std::map<id_t,const Transform*>>& GetAsyncUpdatedTransforms() {
        return instance->async_updated_transforms;
    }

    // Inherited from Parse
    virtual bool Serialize(rapidjson::Document& document);

    // Inherited from Parse
    virtual bool Parse(rapidjson::Value& node);
private:

    friend class Transform;
    friend class physics::PhysicsSystem;

    static AtomicMap<id_t,const Transform*>& GetUpdatedTransforms() {
        return instance->updated_transforms;
    };

    std::map<unsigned int, std::shared_ptr<Transform>> transforms;

    AtomicMap<id_t,const Transform*> updated_transforms;
    AsyncData<std::map<id_t,const Transform*>> async_updated_transforms;
};

} // End of trillek

#endif
