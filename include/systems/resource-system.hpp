#ifndef RESOURCE_SYSTEM_HPP_INCLUDED
#define RESOURCE_SYSTEM_HPP_INCLUDED

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include "property.hpp"
#include "trillek.hpp"
#include "util/json-parser.hpp"

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

// Singleton approach derived from http://silviuardelean.ro/2012/06/05/few-singleton-approaches/ .
class ResourceMap : public util::Parser {
private:
    ResourceMap() : Parser("resources") { }
    ResourceMap(const ResourceMap& right) : Parser("resources") {
        instance = right.instance;
    }
    ResourceMap& operator=(const ResourceMap& right) {
        if (this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<ResourceMap> instance;
public:
    static std::shared_ptr<ResourceMap> GetInstance() {
        std::call_once(ResourceMap::only_one,
            [ ] () {
            ResourceMap::instance.reset(new ResourceMap());

                // Set up the default factory for an unknown type to return false.
                auto lambda = [ ] (const std::string& name, const std::vector<Property> &properties) {
                    return nullptr;
                };

                ResourceMap::instance->factories[""] = lambda;

                ResourceMap::instance->RegisterTypes();
            }
        );

        return ResourceMap::instance;
    }
    ~ResourceMap() { }

    /**
     * \brief Register a type to be available for factory calls.
     *
     * \return void
     */
    template<class T>
    static void RegisterResourceType() {
        // Store the type ID associated with the type name.
        resource_type_id[reflection::GetTypeName<T>()] = reflection::GetTypeID<T>();

        // Create a lambda function that calls Create with the correct template type.
        auto lambda = [] (const std::string& name, const std::vector<Property> &properties) {
            return instance->Create<T>(name, properties);
        };

        instance->factories[reflection::GetTypeName<T>()] = lambda;
    }

    /**
     * \brief Returns a type ID associated with the given name.
     *
     * \param[in] const std::string & type_Name The name to look for a type ID.
     * \return unsigned int Returns 0 if the name doesn't exist.
     */
    static unsigned int GetTypeIDFromName(const std::string& type_Name) {
        if (resource_type_id.find(type_Name) == resource_type_id.end()) {
            return 0;
        }
        return resource_type_id.find(type_Name)->second;
    }

    /**
     * \brief Gets a resource by the given name.
     *
     * \param[in] const std::string& name Name of the resource to retrieve.
     * \return std::shared_ptr<T> Returns nullptr if the resource hasn't been created yet, otherwise the requested resource..
     */
    template<class T>
    static std::shared_ptr<T> Get(const std::string& name) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (instance->resources[type_id].find(name) == instance->resources[type_id].end()) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(instance->resources[type_id][name]);
    }

    /**
     * \brief Used to create a resource at runtime when type information isn't available, but the type ID is.
     *
     * This also differs with the compile time version in that it can't return the created resource, but it can
     * only return wether it was created successfully or not. This is because you can't template the retrun value
     * if you don't have the type information.
     * \param[in] const std::string type_name The string name of the type of resource to create. This is used to select the correct factory.
     * \param[in] const std::string & name What the created resource will be named.
     * \param[in] const std::vector<Property> & properties The creation properties for the resource.
     * \return bool True if the resource loaded successfully. Get must be used, later, where the type information is known to retrieve the resource if it was loaded correctly.
     */
    static bool Create(const std::string type_name, const std::string& name, const std::vector<Property> &properties) {
        if (instance->factories.find(type_name) != instance->factories.end()) {
            return instance->factories[type_name](name, properties) != nullptr;
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
    static std::shared_ptr<T> Create(const std::string& name, const std::vector<Property> &properties) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (instance->resources[type_id].find(name) == instance->resources[type_id].end()) {
            instance->resources[type_id][name] = std::make_shared<T>();
            if (!instance->resources[type_id][name]->Initialize(properties)) {
                instance->resources[type_id].erase(name);
                return nullptr;
            }
        }
        return std::static_pointer_cast<T>(instance->resources[type_id][name]);
    }

    /**
     * \brief Adds a resource to be managed by the system.
     *
     * \param[in] const std::string & name The name of the resource.
     * \param[in] std::shared_ptr<T> r The resource to add.
     * \return void
     */
    template<class T>
    static void Add(const std::string& name, std::shared_ptr<T> r) {
        unsigned int type_id = reflection::GetTypeID<T>();
        instance->resources[type_id][name] = r;
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
    static void Remove(const std::string& name) {
        for (const auto& list : instance->resources) {
            if (list.second.find(name) != list.second.end()) {
                instance->resources[list.first].erase(name);
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
    static bool Exists(const std::string& name) {
        for (const auto& list : instance->resources) {
            if (list.second.find(name) != list.second.end()) {
                return true;
            }
        }
        return false;
    }

    /**
     * \brief Registers all resource types as defined in the function body.
     *
     * This function is defined in a separate source file to reduce compile times.
     * This function is coupled to each resource type, and all resource types
     * known at compile time should be registered via the function body.
     * Internally it just calls the template method Register().
     * \return void
     */
    static void RegisterTypes();

    // Inherited from Parse
    virtual bool Serialize(rapidjson::Document& document);

    // Inherited from Parse
    virtual bool Parse(rapidjson::Value& node);
private:
    static std::map<unsigned int, std::map<std::string, std::shared_ptr<ResourceBase>>> resources; // Mapping of resource TypeID to loaded resources
    static std::map<std::string, unsigned int> resource_type_id; // Stores a mapping of TypeName to TypeID
    static std::map<std::string, std::function<std::shared_ptr<ResourceBase>(const std::string& name, const std::vector<Property> &properties)>> factories; // Mapping of type ID to factory function.
};

} // End of system
} // End of trillek

#endif
