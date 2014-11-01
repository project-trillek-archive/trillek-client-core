#ifndef COMMPONENT_FACTORY_HPP_INCLUDED
#define COMMPONENT_FACTORY_HPP_INCLUDED

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <vector>
#include "property.hpp"
#include "trillek.hpp"
#include "component-templates.hpp"
#include "component-adder.hpp"
#include "systems/system-base.hpp"
#include "util/json-parser.hpp"
#include "systems/rewindable-map.hpp"
#include "logging.hpp"

namespace trillek {

using component::Component;

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
    template<class T>
    using commit = std::map<id_t,const std::shared_ptr<const T>,std::less<id_t>,
                         TrillekAllocator<std::pair<const id_t,std::shared_ptr<const T>>>>;

    template<class T>
    using history = HistoryMap<commit<T>>;

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
    template<ComponentType cptype,Component C,class T>
    void RegisterComponentType(ComponentAdder<cptype,C,T>&& adder) {
        // Store the type ID associated with the type name.
        LOGMSGC(DEBUG) << "adding factory of " << reflection::GetTypeName<std::integral_constant<Component,C>>();
        component_type_id[reflection::GetTypeName<std::integral_constant<Component,C>>()] = static_cast<uint32_t>(C);

        instance->factories[static_cast<uint32_t>(C)] =
            std::bind(&ComponentAdder<cptype,C,T>::Create, adder, std::placeholders::_1, std::placeholders::_2);
    }

    template<class T>
    void RegisterComponentType() {
        // Store the type ID associated with the type name.
        component_type_id[reflection::GetTypeName<T>()] = reflection::GetTypeID<T>();

        // Create a lambda function that calls Create with the correct template type.
        // This will also add the component to the correct system.
        auto lambda = [] (const unsigned int entity_id, const std::vector<Property> &properties) {
            auto comp = instance->Create<T>(entity_id, properties);

            unsigned int type_id = reflection::GetTypeID<T>();
            if (comp && (instance->systems.find(type_id) != instance->systems.end())) {
                instance->systems.at(type_id)->AddDynamicComponent(entity_id, comp);
                return true;
            }

            return false;
        };

        instance->factories[reflection::GetTypeID<T>()] = lambda;
    }


    /**
     * \brief Register a type to be available for factory dynamic calls.
     *
     * \return void
     */
    template<Component C>
    void RegisterComponentType() {
        // Store the type ID associated with the type name.
        component_type_id[reflection::GetTypeName<std::integral_constant<Component,C>>()] = static_cast<uint32_t>(C);

        // Create a lambda function that calls Create with the correct template type.
        // This will also add the component to the correct system.
        auto lambda =
        [] (const unsigned int entity_id, const std::vector<Property> &properties) {
            auto inst = GetInstance();
            auto comp = inst->Create<C>(entity_id, properties);

            const unsigned int type_id = static_cast<uint32_t>(C);
            if (comp && (inst->systems.count(type_id))) {
                (ComponentAdder<DYNAMIC,C,SystemBase>(inst->systems.at(type_id)))
                                                                    (entity_id, comp);
                return true;
            }
            return false;
        };

        instance->factories[static_cast<uint32_t>(C)] = lambda;
    }

    /**
     * \brief Register a system to be available for component registration.
     *
     * \param[in] SystemBase* system The system to add for the template's type ID.
     * \return void
     */
    template<class T>
    void RegisterSystem(SystemBase* system) {
        instance->systems[reflection::GetTypeID<T>()] = system;
    }

    /**
     * \brief Returns a type ID associated with the given name.
     *
     * \param[in] const std::string & type_Name The name to look for a type ID.
     * \return unsigned int Returns 0 if the name doesn't exist.
     */
    unsigned int GetTypeIDFromName(const std::string& type_Name) {
        if (! component_type_id.count(type_Name)) {
            LOGMSGC(ERROR) << "Could not find id type of " << type_Name;
            return 0;
        }
        return component_type_id.find(type_Name)->second;
    }

    /**
     * \brief Gets a component for the given entity ID.
     *
     * The type of component to retrieve is determined by the template's type ID.
     * \param[in] const std::string& name Name of the component to retrieve.
     * \return std::shared_ptr<T> Returns nullptr if the component hasn't been
     * created yet, otherwise the requested component..
     */
    template<class T>
    std::shared_ptr<T> Get(id_t entity_id) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (! instance->graphic_components[type_id].count(entity_id)) {
            LOGMSGC(ERROR) << "Component type #" << type_id <<
                                    " not found for entity id #" << entity_id;
            return nullptr;
        }
        return std::static_pointer_cast<T>(instance->graphic_components[type_id].at(entity_id));
    }

    /**
     * \brief Used to create a component at runtime when type information isn't
     * available, but the type ID is.
     *
     * This also differs with the compile time version in that it can't return
     * the created component, but it can only return wether it was created
     * successfully or not. This is because you can't template the retrun value
     * if you don't have the type information.
     * \param[in] const unsigned int type_id The ID of the type of component to
     * create. This is used to select the correct factory.
     * \param[in] const std::string & name What the created component will be named.
     * \param[in] const std::vector<Property> & properties The creation
     * properties for the component.
     * \return bool True if the component loaded successfully. Get must be used,
     * later, where the type information is known to retrieve the component if
     * it was loaded correctly.
     */
    bool Create(const unsigned int type_id, const unsigned int entity_id,
                                    const std::vector<Property> &properties) {
        if (instance->factories.count(type_id)) {
            return instance->factories[type_id](entity_id, properties);
        }
        return false;
    }

    /**
     * \brief Creates a component with the given name and initializes it. This
     * is used at compile time when type information is known.
     *
     * Only used by the graphics system.
     *
     * \param[in] const std::string & name The name of the component to create.
     * \param[in] const std::vector<Property> & properties The creation
     * properties for the component.
     * \return std::shared_ptr<T> Returns nullptr if it failed to be created or
     * already exists, otherwise the created component.
     */
    template<class T>
    std::shared_ptr<graphics::Container> Create(const unsigned int entity_id, const std::vector<Property> &properties) {
        unsigned int type_id = reflection::GetTypeID<T>();
        if (instance->graphic_components[type_id].find(entity_id) == instance->graphic_components[type_id].end()) {
            auto sharedcomp = std::make_shared<graphics::Container>(T());
            sharedcomp->Get<T>().component_type_id = type_id;
            if (!sharedcomp->Get<T>().Initialize(properties)) {
                instance->graphic_components[type_id].erase(entity_id);
                return nullptr;
            }
            instance->graphic_components[type_id][entity_id] = sharedcomp;
            return std::move(sharedcomp);
        }
        return instance->graphic_components[type_id][entity_id];
    }

    /**
     * \brief Adds a component to be managed by the system.
     *
     * \param[in] const std::string & name The name of the component.
     * \param[in] std::shared_ptr<const T> r The component to add.
     * \return void
     */
    template<class T,class U = std::shared_ptr<const T>>
    void Add(id_t entity_id, U&& r) {
        instance->graphic_components.at(reflection::GetTypeID<T>())[entity_id] = std::forward<U>(r);
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
    void RegisterTypes();

    // Inherited from Parse
    bool Serialize(rapidjson::Document& document);

    // Inherited from Parse
    virtual bool Parse(rapidjson::Value& node);
private:
    std::map<unsigned int, std::map<id_t, std::shared_ptr<graphics::Container>>> graphic_components; // Mapping of component TypeID to loaded components for graphic
    std::map<unsigned int, SystemBase*> systems; // Mapping of component TypeID to system to add it to
    std::map<std::string, unsigned int> component_type_id; // Stores a mapping of TypeName to TypeID
    std::map<unsigned int, std::function<bool(const unsigned int, const std::vector<Property> &properties)>> factories; // Mapping of type ID to factory function.
};

} // End of trillek

#endif
