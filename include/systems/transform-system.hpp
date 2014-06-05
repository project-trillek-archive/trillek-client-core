#ifndef TRANSFORMSYSTEM_HPP_INCLUDED
#define TRANSFORMSYSTEM_HPP_INCLUDED

#include <memory>
#include <map>
#include <mutex>

#include "systems/json-parser.hpp"

namespace trillek {
namespace transform {

class Transform;

// Stores a mapping of entity ID to transform that can
// be accessed via static methods anywhere.
class TransformMap : public json::SerializerBase {
private:
    TransformMap() : SerializerBase("transforms") { }
    TransformMap(const TransformMap& right) : SerializerBase("transforms") {
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

    // Inherited from SerializeBase
    virtual bool Serialize(rapidjson::Document& document);

    // Inherited from SerializeBase
    virtual bool DeSerialize(rapidjson::Value& node);
private:
    std::map<unsigned int, std::shared_ptr<Transform>> transforms;
};

} // End of transform
} // End of trillek

#endif
