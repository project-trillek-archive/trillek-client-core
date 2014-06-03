#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <string>

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
    };


    /**
      * \brief A generic value holder type.
      *
      */
    template <typename t>
    class ValueHolder : public ValueHolderBase {
    public:
        ValueHolder(t value) : value(value) { }
        virtual ValueHolder* Clone() const { return new ValueHolder(value); }
        t Get() { return this->value; }
    private:
        t value;
    };

    std::string name; // Name of this property.
    ValueHolderBase* value_holder; // The value held by this property.
};

#endif
