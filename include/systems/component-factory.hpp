#ifndef COMMPONENT_FACTORY_HPP_INCLUDED
#define COMMPONENT_FACTORY_HPP_INCLUDED

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include "property.hpp"
#include "trillek.hpp"
#include "component.hpp"
#include "systems/system-base.hpp"
#include "util/json-parser.hpp"

namespace trillek {

// Singleton approach derived from http://silviuardelean.ro/2012/06/05/few-singleton-approaches/ .
class ComponentFactory : public util::Parser {
private:
    ComponentFactory() : Parser("entities") { }
    ComponentFactory(const ComponentFactory& right) : Parser("entities") {
        instance = right.instance;
    }
    ComponentFactory& operator=(const ComponentFactory& right) {
        if (this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<ComponentFactory> instance;
public:
    static std::shared_ptr<ComponentFactory> GetInstance() {
        std::call_once(ComponentFactory::only_one,
            [ ] () {
            ComponentFactory::instance.reset(new ComponentFactory());

                // Set up the default factory for an unknown type to return false.
                auto lambda = [ ] (const unsigned int, const std::vector<Property> &properties) {
                    return nullptr;
                };

                ComponentFactory::instance->factories[0] = lambda;

                ComponentFactory::instance->RegisterTypes();
            }
        );

        return ComponentFactory::instance;
    }
    ~ComponentFactory() { }

    /**
     * \brief Register a type to be available for factory calls.
     *
     * \return void
     */
    template<class T>
    static void RegisterComponentType() {
        // Store the type ID associated with the type name.
        component_type_id[reflection::GetTypeName<T>()] = reflection::GetTypeID<T>();

        // Create a lambda function that calls Create with the correct template type.
        // This will also add the component to the correct system.
        auto lambda = [ ] (const unsigned int entity_id, const std::vector<Property> &properties) {
            auto comp = instance->Create<T>(entity_id, properties);

            unsigned int type_id = reflection::GetTypeID<T>();
            if (comp && (instance->systems.find(type_id) != instance->systems.end())) {
                instance->systems.at(type_id)->AddComponent(entity_id, comp);
            }

            return comp;
        };

        instance->factories[reflection::GetTypeID<T>()] = lambda;
    }

    /**
     * \brief Register a system to be available for component registration.
     *
     * \param[in] SystemBase* system The system to add for the template's type ID.
     * \return void
     */
    template<class T>
    static void RegisterSystem(SystemBase* system) {
        instance->systems[reflection::GetTypeID<T>()] = system;
    }

    /**
     * \brief Returns a type ID associated with the given name.
     *
     * \param[in] const std::string & type_Name The name to look for a type ID.
     * \return unsigned int Returns 0 if the name doesn't exist.
     */
    static unsigned int GetTypeIDFromName(const std::string& type_Name) {
        if (component_type_id.find(type_Name) == component_type_id.end()) {
            return 0;
        }
        return component_type_id.find(type_Name)->second;
    }

    /**
     * \brief Gets a component for the given entity ID.
     *
     * The type of component to retrieve is determined by the template's type ID.
     * \param[in] const std::string& name Name of the component to retrieve.
     * \return std::shared_ptr<T> Returns nullptr if the component hasn't been created yet, otherwise the requested component..
     */
    template<class T>
    static std::shared_ptr<T> Get(const unsigned int entity_id) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (instance->components[type_id].find(entity_id) == instance->components[type_id].end()) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(instance->components[type_id][entity_id]);
    }

    /**
     * \brief Used to create a component at runtime when type information isn't available, but the type ID is.
     *
     * This also differs with the compile time version in that it can't return the created component, but it can
     * only return wether it was created successfully or not. This is because you can't template the retrun value
     * if you don't have the type information.
     * \param[in] const unsigned int type_id The ID of the type of component to create. This is used to select the correct factory.
     * \param[in] const std::string & name What the created component will be named.
     * \param[in] const std::vector<Property> & properties The creation properties for the component.
     * \return bool True if the component loaded successfully. Get must be used, later, where the type information is known to retrieve the component if it was loaded correctly.
     */
    static bool Create(const unsigned int type_id, const unsigned int entity_id, const std::vector<Property> &properties) {
        if (instance->factories.find(type_id) != instance->factories.end()) {
            return instance->factories[type_id](entity_id, properties) != nullptr;
        }
        return false;
    }

    /**
     * \brief Creates a component with the given name and initializes it. This is used at compile time when type information is known.
     *
     * \param[in] const std::string & name The name of the component to create.
     * \param[in] const std::vector<Property> & properties The creation properties for the component.
     * \return std::shared_ptr<T> Returns nullptr if it failed to be created, otherwise the created component.
     */
    template<class T>
    static std::shared_ptr<T> Create(const unsigned int entity_id, const std::vector<Property> &properties) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (instance->components[type_id].find(entity_id) == instance->components[type_id].end()) {
            auto sharedcomp = std::make_shared<T>();
            sharedcomp->component_type_id = type_id;
            if (!sharedcomp->Initialize(properties)) {
                instance->components[type_id].erase(entity_id);
                return nullptr;
            }
            instance->components[type_id][entity_id] = sharedcomp;
            return sharedcomp;
        }
        return std::static_pointer_cast<T>(instance->components[type_id][entity_id]);
    }

    /**
     * \brief Adds a component to be managed by the system.
     *
     * \param[in] const std::string & name The name of the component.
     * \param[in] std::shared_ptr<T> r The component to add.
     * \return void
     */
    template<class T>
    static void Add(const unsigned int entity_id, std::shared_ptr<T> r) {
        unsigned int type_id = reflection::GetTypeID<T>();
        instance->components[type_id][entity_id] = r;
    }

    /**
     * \brief Removes a components managed by the system.
     *
     * This doesn't invalidate any strong references to the component as it doesn't destroy the component.
     * Any weak references to the component should be checked to make sure they are valid and haven't been
     * removed.
     * \param[in] const std::string & name Name of the component to remove.
     * \return void
     */
    static void Remove(const unsigned int entity_id) {
        for (const auto& list : instance->components) {
            if (list.second.find(entity_id) != list.second.end()) {
                instance->components[list.first].erase(entity_id);
                return;
            }
        }
    }

    /**
     * \brief Checks if a component exists with the given name.
     *
     * \param[in] const std::string& name Name of the component to check if it exists.
     * \return bool True if the component exists.
     */
    static bool Exists(const unsigned int entity_id) {
        for (const auto& list : instance->components) {
            if (list.second.find(entity_id) != list.second.end()) {
                return true;
            }
        }
        return false;
    }

    /**
     * \brief Registers all component types as defined in the function body.
     *
     * This function is defined in a separate source file to reduce compile times.
     * This function is coupled to each component type, and all component types
     * known at compile time should be registered via the function body.
     * Interally it just calls the tempalte method Register().
     * \return void
     */
    static void RegisterTypes();

    // Inherited from Parse
    virtual bool Serialize(rapidjson::Document& document);

    // Inherited from Parse
    virtual bool Parse(rapidjson::Value& node);
private:
    static std::map<unsigned int, std::map<unsigned int, std::shared_ptr<ComponentBase>>> components; // Mapping of component TypeID to loaded components
    static std::map<unsigned int, SystemBase*> systems; // Mapping of component TypeID to system to add it to
    static std::map<std::string, unsigned int> component_type_id; // Stores a mapping of TypeName to TypeID
    static std::map<unsigned int, std::function<std::shared_ptr<ComponentBase>(const unsigned int, const std::vector<Property> &properties)>> factories; // Mapping of type ID to factory function.
};

} // End of trillek

#endif
