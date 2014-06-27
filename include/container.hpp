#ifndef CONTAINER_HPP_INCLUDED
#define CONTAINER_HPP_INCLUDED

#include "trillek.hpp"

namespace trillek {

/**
 * \brief A class to contain an arbitrary value.
 *
 * This class is used to carry dynamic typed values.
 * The type contained can be determined with GetType() or Is<T>()
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
    Container(const Container &that) = delete;
    Container& operator=(const Container &that) = delete;

    // Move
    Container(Container&& that) : value_holder(that.value_holder) {
        that.value_holder = nullptr;
    }
    Container& operator=(Container&& that) {
        value_holder = that.value_holder;
        that.value_holder = nullptr;
        return *this;
    }

    /**
     * \brief Sets the value of the container.
     */
    template <typename T>
    Container(T value) : value_holder(new ValueHolder<T>(value)) { }

    /**
     * \brief Retrieves the container value.
     */
    template <class T>
    T Get() const {
        if(this->value_holder != nullptr) {
            return static_cast<ValueHolder<T>*>(this->value_holder)->Get();
        }
    }

    bool IsEmpty() const {
        return nullptr == this->value_holder;
    }

    /**
     * \brief Compares the type of contained value.
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
     * \brief Retrieves the type ID of contained value.
     */
    unsigned GetType() const {
        if(this->value_holder == nullptr) {
            return 0;
        }
        return this->value_holder->GetType();
    }

    /**
     * \brief Retrieves the size of the contained value.
     */
    std::size_t GetSize() const {
        if(this->value_holder == nullptr) {
            return 0;
        }
        return this->value_holder->GetSize();
    }

private:
    /**
     * \brief ValueHolderBase - a common type class
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
    template <typename T>
    class ValueHolder : public ValueHolderBase {
    public:
        ValueHolder(T value) : value(value) { }
        T Get() { return this->value; }
        virtual unsigned GetType() const { return reflection::GetTypeID<T>(); }
        virtual std::size_t GetSize() const { return sizeof(T); }
    private:
        T value;
    };

    ValueHolderBase* value_holder; // The value held.
};

} // namespace trillek

#endif
