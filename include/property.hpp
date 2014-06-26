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
     * \param[in] t value The value of the property. typename is inferred on usage.
     */
    template <typename t>
    Property(std::string name, t value) : name(name), value_holder(new ValueHolder<t>(value)) { }

    ~Property() { delete this->value_holder; }

    /**
     * \brief Retrieves the value in value_holder.
     *
     * Calls the Get() method of ValueHolder with the given template type.
     * \returns t The value with the given template type.
     */
    template <typename t>
    t Get() const { return static_cast<ValueHolder<t>*>(this->value_holder)->Get(); }

    /**
     * \brief Gets the name of this property.
     *
     * \returns std::string The name of this property.
     */
    std::string GetName() const { return this->name; }

    /**
     * \brief Compares the type ID of value_holder contents to the template type.
     *
     * Calls the GetType() method of ValueHolder.
     * \returns bool true if the IDs match and are non-zero
     */
    template <class CT>
    bool Is() const {
        unsigned type_id = GetType();
        if(type_id) {
            return reflection::GetTypeID<CT>() == type_id;
        }
        else {
            return false;
        }
    }

    /**
     * \brief Retrieves the type ID of value_holder contents.
     *
     * Calls the GetType() method of ValueHolder.
     * \returns unsigned int the type ID.
     */
    unsigned GetType() const {
        if(this->value_holder == nullptr) {
            return 0;
        }
        return this->value_holder->GetType();
    }

    /**
     * \brief Retrieves the size of the value_holder contents.
     *
     * Calls the GetSize() method of ValueHolder.
     * \returns size_t The size of the value in the container.
     */
    std::size_t GetSize() const {
        if(this->value_holder == nullptr) {
            return 0;
        }
        return this->value_holder->GetSize();
    }
private:
    /**
     * \brief ValueHolderBase is a common base type that can be used to holder a pointer to a specialized templated version of ValueHolder.
     *
     */
    class ValueHolderBase {
    public:
        virtual ~ValueHolderBase() { }
        /**
         * \brief Creates a clone of the held object.
         *
         * \returns ValueHolderBase* A clone of the held object.
         */
        virtual ValueHolderBase* Clone() const = 0;
        virtual unsigned GetType() const { return 0; }
        virtual std::size_t GetSize() const { return 0; }
    };


    /**
     * \brief A generic value holder type.
     */
    template <typename t>
    class ValueHolder : public ValueHolderBase {
    public:
        ValueHolder(t value) : value(value) { }
        virtual ValueHolder* Clone() const { return new ValueHolder(value); }
        t Get() { return this->value; }
        virtual unsigned GetType() const { return reflection::GetTypeID<t>(); }
        virtual std::size_t GetSize() const { return sizeof(t); }
    private:
        t value;
    };

    std::string name; // Name of this property.
    ValueHolderBase* value_holder; // The value held by this property.
};

} // namespace trillek

#endif
