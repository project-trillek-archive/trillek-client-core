#ifndef TRANSFORMSYSTEM_HPP_INCLUDED
#define TRANSFORMSYSTEM_HPP_INCLUDED

#include <memory>
#include <map>
#include <mutex>

#include "systems/json-parser.hpp"

namespace trillek {
namespace resource {

class Transform;

} // End of resource

namespace system {

class TransformSystem : public SerializerBase {
private:
    TransformSystem() : SerializerBase("transforms") { }
    TransformSystem(const TransformSystem& right) : SerializerBase("transforms") {
        instance = right.instance;
    }
    TransformSystem& operator=(const TransformSystem& right) {
        if (this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<TransformSystem> instance;
public:
    static std::shared_ptr<TransformSystem> GetInstance() {
        std::call_once(TransformSystem::only_one,
            [ ] () {
                TransformSystem::instance.reset(new TransformSystem());

                system::JSONParser::RegisterSerializer(TransformSystem::instance);
            }
        );

        return TransformSystem::instance;
    }
    ~TransformSystem() { }

    /**
    * \brief Gets an entity's transform.
    *
    * \param[in] const unsigned int entity_id ID of the entity to get the transform for.
    * \return std::shared_ptr<resource::Transform> The entity's transofmr (nullptr if no transform found).
    */
    static std::shared_ptr<resource::Transform> AddTransform(const unsigned int entity_id);

    /**
    * \brief Gets an entity's transform.
    *
    * \param[in] const unsigned int entity_id ID of the entity to get the transform for.
    * \return std::shared_ptr<resource::Transform> The entity's transofmr (nullptr if no transform found).
    */
    static std::shared_ptr<resource::Transform> GetTransform(const unsigned int entity_id);

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
    std::map<unsigned int, std::shared_ptr<resource::Transform>> transforms;
};

} // End of system
} // End of trillek

#endif
