#ifndef CONTAINER_HPP_INCLUDED
#define CONTAINER_HPP_INCLUDED

#include "trillek.hpp"
#include "type-id.hpp"

namespace trillek {
namespace graphics {

/**
 * \brief A class to contain an arbitrary value.
 *
 * This class is used to carry dynamic typed values.
 * The type contained can be determined with GetType() or Is<T>()
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
     * \brief Retrieves the container value by reference.
     */
    template <class T>
    T& Get() {
        return static_cast<ValueHolder<T>*>(this->value_holder)->Get();
    }

    /**
     * \brief Retrieves the container value by const reference.
     */
    template <class T>
    const T& Get() const {
        return static_cast<ValueHolder<T>*>(this->value_holder)->Get();
    }

    /** \brief Create an alias to a shared pointer
     *
     * \param ct const std::shared_ptr<Container>& the original pointer
     * \return std::shared_ptr<T> the alias
     *
     */
    template<class T>
    static std::shared_ptr<T> GetSharedPtr(const std::shared_ptr<Container>& ct) {
        return std::shared_ptr<T>(ct,&ct->Get<T>());
    }

    bool IsEmpty() const {
        return nullptr == this->value_holder;
    }

    /**
     * \brief Compares the type of contained value (if known).
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
     * \brief ValueHolderBase - common class
     */
    class ValueHolderBase {
    public:
        virtual ~ValueHolderBase() { }
        virtual unsigned GetType() const { return 0; }
        virtual std::size_t GetSize() const { return 0; }
    };

    /**
     * \brief The generic value holder.
     */
    template <typename T>
    class ValueHolder : public ValueHolderBase {
    public:
        ValueHolder(T value) : value(value), type_id(reflection::GetTypeID<T>()) { }
        T& Get() { return this->value; }
        virtual unsigned GetType() const { return type_id; }
        virtual std::size_t GetSize() const { return sizeof(T); }
    private:
        unsigned type_id;
        T value;
    };

    ValueHolderBase* value_holder;
};

} // namespace graphics
} // namespace trillek

#endif
