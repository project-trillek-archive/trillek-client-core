#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <string>
#include "trillek.hpp"

namespace trillek {
/**
 * \brief A class to contain a generic property.
 *
 * This class is used to pass around generic properties.
 * Properties have a name and a value. The value is stored in
 * value_holder and is accessed by calling Get() with the
 * appropriate type.
 */
class Property {
private:
    Property() { }
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
     * \param[in] T value The value of the property. typename is inferred.
     */
    template <typename T>
    Property(std::string name, T value) : name(name), value_holder(new ValueHolder<T>(value)) { }

    ~Property() { delete this->value_holder; }

    /**
     * \brief Retrieves the value in value_holder.
     */
    template <typename T>
    T Get() const { return static_cast<ValueHolder<T>*>(this->value_holder)->Get(); }

    /**
     * \brief Gets the name of this property.
     */
    std::string GetName() const { return this->name; }

    /**
     * \brief Compares the type contents.
     * \returns true if match
     */
    template <class T>
    bool Is() const {
        unsigned type_id = GetType();
        if(type_id) {
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
        ValueHolder(T value) : value(value) { }
        virtual ValueHolder* Clone() const { return new ValueHolder(value); }
        T Get() { return this->value; }
        virtual unsigned GetType() const { return reflection::GetTypeID<T>(); }
        virtual std::size_t GetSize() const { return sizeof(T); }
    private:
        T value;
    };

    std::string name; // Name of this property.
    ValueHolderBase* value_holder; // The value held.
};

} // namespace trillek

#endif
