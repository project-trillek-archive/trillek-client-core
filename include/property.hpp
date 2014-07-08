#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <string>
#include "trillek.hpp"
#include "type-id.hpp"

namespace trillek {
/**
 * \brief A class to contain a generic property.
 *
 * This class is used to pass around generic properties.
 * Properties have a name and a value. The value is
 * accessed by calling Get() with the appropriate type.
 */
class Property {
private:
    Property() : value_holder(nullptr) { }
public:
    // Copy
    Property(const Property &other) {
        this->name = other.name;
        if (other.value_holder != nullptr) {
            this->value_holder = other.value_holder->Clone();
        }
        else {
            this->value_holder = nullptr;
        }
    }

    // Move
    Property(Property&& other) : name(other.name), value_holder(other.value_holder) {
        other.value_holder = nullptr;
    }

    /**
     * \brief Sets the name and value of the property.
     *
     * \param[in] std::string name The name of the property
     * \param[in] T value The value of the property.
     */
    template <typename T>
    Property(std::string name, T value) : name(name), value_holder(new ValueHolder<T>(value)) { }

    ~Property() { delete this->value_holder; }

    template <typename T>
    T Get() const { return static_cast<ValueHolder<T>*>(this->value_holder)->Get(); }

    /**
     * \brief Gets the property name.
     */
    std::string GetName() const { return this->name; }

    /**
     * \brief Compares the type contents.
     * \returns true if match
     */
    template <class T>
    bool Is() const {
        unsigned type_id = GetType();
        if(type_id && type_id != ~0) {
            return reflection::GetTypeID<T>() == type_id;
        }
        else {
            return false;
        }
    }

    /**
     * \brief Retrieves the type ID of contents.
     */
    unsigned GetType() const {
        if(this->value_holder == nullptr) {
            return 0;
        }
        return this->value_holder->GetType();
    }

    std::size_t GetSize() const {
        if(this->value_holder == nullptr) {
            return 0;
        }
        return this->value_holder->GetSize();
    }
private:
    class ValueHolderBase {
    public:
        virtual ~ValueHolderBase() { }
        virtual ValueHolderBase* Clone() const = 0;
        virtual unsigned GetType() const { return 0; }
        virtual std::size_t GetSize() const { return 0; }
    };

    /**
     * \brief A generic value holder type.
     */
    template <typename T>
    class ValueHolder : public ValueHolderBase {
    public:
        ValueHolder(T value) : value(value), type_id(reflection::GetTypeID<T>()) { }
        virtual ValueHolder* Clone() const { return new ValueHolder(value); }
        T Get() { return this->value; }
        virtual unsigned GetType() const { return type_id; }
        virtual std::size_t GetSize() const { return sizeof(T); }
    private:
        unsigned type_id;
        T value;
    };

    std::string name;
    ValueHolderBase* value_holder;
};

} // namespace trillek

#endif
