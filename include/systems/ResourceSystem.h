#ifndef RESOURCESYSTEM_H_INCLUDED
#define RESOURCESYSTEM_H_INCLUDED

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include "Property.h"
#include "Trillek.h"
#include "systems/json-parser.hpp"

namespace trillek {
namespace resource {

class ResourceBase {
public:
    ResourceBase() { }
    ~ResourceBase() { }

    /**
    * \brief Returns a resource with the specified name.
    *
    * \param[in] const std::vector<Property> &properties The creation properties for the resource.
    * \return bool True if initialization finished with no errors.
    */
    virtual bool Initialize(const std::vector<Property> &properties) = 0;
};

} //End of resource

namespace system {
// Singleton approach derived from http://silviuardelean.ro/2012/06/05/few-singleton-approaches/ .
class ResourceSystem : public system::SerializerBase {
private:
    ResourceSystem() : SerializerBase("resources") { }
    ResourceSystem(const ResourceSystem& right) : SerializerBase("resources") {
        instance = right.instance;
    }
    ResourceSystem& operator=(const ResourceSystem& right) {
        if (this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<ResourceSystem> instance;
public:
    static std::shared_ptr<ResourceSystem> GetInstance() {
        std::call_once(ResourceSystem::only_one,
            [ ] () {
                ResourceSystem::instance.reset(new ResourceSystem());

                // Set up the default factory for an unknown type to return false.
                auto lambda = [ ] (const std::string& name, const std::vector<Property> &properties) {
                    return nullptr;
                };

                ResourceSystem::instance->factories[0] = lambda;

                system::JSONParser::RegisterSerializer(ResourceSystem::instance);
            }
        );

        return ResourceSystem::instance;
    }
    ~ResourceSystem() { }

    /**
    * \brief Register a type to be available for factory calls.
    *
    * \return void
    */
    template<class T>
    void Register() {
        // Store the type ID associated with the type name.
        this->resource_type_id[reflection::GetTypeName<T>()] = reflection::GetTypeID<T>();

        // Create a lambda function that calls Create with the correct template type.
        auto lambda = [this] (const std::string& name, const std::vector<Property> &properties) {
            return this->Create<T>(name, properties);
        };

        this->factories[reflection::GetTypeID<T>()] = lambda;
    }

    /**
    * \brief Returns a type ID associated with the given name.
    *
    * \param[in] const std::string & type_Name The name to look for a type ID.
    * \return unsigned int Returns 0 if the name doesn't exist.
    */
    unsigned int GetTypeIDFromName(const std::string& type_Name) {
        if (this->resource_type_id.find(type_Name) == this->resource_type_id.end()) {
            return 0;
        }
        return this->resource_type_id.find(type_Name)->second;
    }

    /**
    * \brief Gets a resource by the given name.
    *
    * \param[in] const std::string& name Name of the resource to retrieve.
    * \return std::shared_ptr<T> Returns nullptr if the resource hasn't been created yet, otherwise the requested resource..
    */
    template<class T>
    std::shared_ptr<T> Get(const std::string& name) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (this->resources[type_id].find(name) == this->resources[type_id].end()) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(this->resources[type_id][name]);
    }

    /**
    * \brief Used to create a resource at runtime when type information isn't available, but the type ID is.
    *
    * This also differs with the compile time version in that it can't return the created resource, but it can
    * only return wether it was created successfully or not. This is because you can't template the retrun value
    * if you don't have the type information.
    * \param[in] const unsigned int type_id The ID of the type of resource to create. This is used to select the correct factory.
    * \param[in] const std::string & name What the created resource will be named.
    * \param[in] const std::vector<Property> & properties The creation properties for the resource.
    * \return bool True if the resource loaded successfully. Get must be used, later, where the type information is known to retrieve the resource if it was loaded correctly.
    */
    bool Create(const unsigned int type_id, const std::string& name, const std::vector<Property> &properties) {
        if (this->factories.find(type_id) != this->factories.end()) {
            return this->factories[type_id](name, properties) != nullptr;
        }
        return false;
    }

    /**
    * \brief Creates a resource with the given name and initializes it. This is used at compile time when type information is known.
    *
    * \param[in] const std::string & name The name of the resource to create.
    * \param[in] const std::vector<Property> & properties The creation properties for the resource.
    * \return std::shared_ptr<T> Returns nullptr if it failed to be created, otherwise the created resource.
    */
    template<class T>
    std::shared_ptr<T> Create(const std::string& name, const std::vector<Property> &properties) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (this->resources[type_id].find(name) == this->resources[type_id].end()) {
            this->resources[type_id][name] = std::make_shared<T>();
            if (!this->resources[type_id][name]->Initialize(properties)) {
                this->resources[type_id].erase(name);
                return nullptr;
            }
        }
        return std::static_pointer_cast<T>(this->resources[type_id][name]);
    }

    /**
    * \brief Adds a resource to be managed by the system.
    *
    * \param[in] const std::string & name The name of the resource.
    * \param[in] std::shared_ptr<T> r The resource to add.
    * \return void
    */
    template<class T>
    void Add(const std::string& name, std::shared_ptr<T> r) {
        unsigned int type_id = reflection::GetTypeID<T>();
        resources[type_id][name] = r;
    }

    /**
    * \brief Removes a resources managed by the system.
    *
    * This doesn't invalidate any strong references to the resource as it doesn't destroy the resource.
    * Any weak references to the resource should be checked to make sure they are valid and haven't been
    * removed.
    * \param[in] const std::string & name Name of the resource to remove.
    * \return void
    */
    void Remove(const std::string& name) {
        for (const auto& list : this->resources) {
            if (list.second.find(name) != list.second.end()) {
                this->resources[list.first].erase(name);
                return;
            }
        }
    }

    /**
    * \brief Checks if a resource exists with the given name.
    *
    * \param[in] const std::string& name Name of the resource to check if it exists.
    * \return bool True if the resource exists.
    */
    bool Exists(const std::string& name) {
        for (const auto& list : this->resources) {
            if (list.second.find(name) != list.second.end()) {
                return true;
            }
        }
        return false;
    }

    // Inherited from SerializeBase
    virtual bool Serialize(rapidjson::Value& node);

    // Inherited from SerializeBase
    virtual bool DeSerialize(rapidjson::Value& node);
private:
    std::map<unsigned int, std::map<std::string, std::shared_ptr<resource::ResourceBase>>> resources; // Mapping of resource TypeID to loaded resources
    std::map<std::string, unsigned int> resource_type_id; // Stores a mapping of TypeName to TypeID

    std::map<unsigned int, std::function<std::shared_ptr<resource::ResourceBase>(const std::string& name, const std::vector<Property> &properties)>> factories; // Mapping of type ID to factory function.
};

} // End of system
} // End of trillek

#endif
