#ifndef CONTAINER_HPP_INCLUDED
#define CONTAINER_HPP_INCLUDED

#include "trillek.hpp"

namespace trillek {

/**
 * \brief A class to contain an arbitrary value.
 *
 * This class is used to carry dynamic typed values.
 * The value is stored in a value_holder and is accessed
 * by calling Get() with the appropriate type.
 * The type contained can be determined with GetType()
 * which returns an enum value matching the type (if known).
 */
class Container {
public:
    // instance an empty container
    Container() : value_holder(nullptr) { }

    // clean up - very important
    ~Container() {
        if(value_holder != nullptr) delete value_holder;
    }
    // Copy is not allowed
    Container(const Container &other) = delete;
    Container& operator=(const Container &other) = delete;

    // Move
    Container(Container&& other) : value_holder(other.value_holder) {
        other.value_holder = nullptr;
    }
    Container& operator=(Container&& other) {
        value_holder = other.value_holder;
        other.value_holder = nullptr;
        return *this;
    }

    /**
     * \brief Sets the value of the container.
     *
     * \param[in] t value The value of the property. typename is inferred on usage.
     */
    template <typename t>
    Container(t value) : value_holder(new ValueHolder<t>(value)) { }

    /**
     * \brief Retrieves the container value.
     *
     * Calls the Get() method of ValueHolder with the given template type.
     * \returns t The value with the given template type.
     */
    template <class t>
    t Get() const {
        if(this->value_holder != nullptr) {
            return static_cast<ValueHolder<t>*>(this->value_holder)->Get();
        }
    }

    bool IsEmpty() const {
        return nullptr == this->value_holder;
    }

    /**
     * \brief Compares the type ID of contained value to the template type.
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
     * \brief Retrieves the type ID of contained value.
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
     * \brief Retrieves the size of the contained value.
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
     * \brief ValueHolderBase is a common base type class
     *
     * it can be used to hold a pointer to a specialized
     * templated version of ValueHolder.
     */
    class ValueHolderBase {
    public:
        virtual ~ValueHolderBase() { }
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
        t Get() { return this->value; }
        virtual unsigned GetType() const { return reflection::GetTypeID<t>(); }
        virtual std::size_t GetSize() const { return sizeof(t); }
    private:
        t value;
    };

    ValueHolderBase* value_holder; // The value held by this property.
};

} // namespace trillek

#endif
