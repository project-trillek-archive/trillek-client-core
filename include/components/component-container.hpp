#ifndef COMPONENT_CONTAINER_HPP_INCLUDED
#define COMPONENT_CONTAINER_HPP_INCLUDED

#include "trillek.hpp"
#include "type-id.hpp"

namespace trillek {
namespace component {

/**
 * \brief A class to contain a component.
 *
 * 'type' contain the integer given to the component
 * in the Component enum.
 *
 * Is() tests the component enumerator
 */
class Container {
public:
    typedef std::underlying_type<Component>::type component_type;

    template<Component C>
    bool Is() const { return static_cast<component_type>(C) == type; };

    component_type GetTypeId() const { return type; };

protected:
    Container(component_type n) : type(n) {};

private:
    component_type type;
};

/**
 * \brief A class to contain the value.of the component
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
class ContainerObject : public Container {
    typedef T content_type;

public:
    template<class U>
    ContainerObject(U&& comp) : Container(static_cast<component_type>(C)), content(std::forward<U>(comp)) {};

    ~ContainerObject() {};

    content_type& Get() { return content; };
    const content_type& Get() const { return content; };

    // Copy is not allowed
    ContainerObject(const ContainerObject &that) = delete;
    ContainerObject& operator=(const ContainerObject &that) = delete;

    // Move
    ContainerObject(ContainerObject&& that) : content(std::move(that.content)) {
    }

    ContainerObject& operator=(ContainerObject&& that) {
        this->content = std::move(that.content);
        return *this;
    }
private:
    content_type content;
};

/** \brief Alias a shared_ptr<Container> to get a shared_ptr<T>
 *
 * \param ct const std::shared_ptr<Container>& the original pointer
 * \return std::shared_ptr<T> the alias
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<T> Get(const std::shared_ptr<Container>& ct) {
    return std::shared_ptr<T>(ct,&std::static_pointer_cast<ContainerObject<C>>(ct)->Get());
}

/** \brief Alias a shared_ptr<const Container> to get a shared_ptr<const T>
 *
 * \param ct const std::shared_ptr<Container>& the original pointer
 * \return std::shared_ptr<const T> the alias
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<const T> Get(const std::shared_ptr<const Container>& ct) {
    return std::shared_ptr<const T>(ct,&std::static_pointer_cast<const ContainerObject<C>>(ct)->Get());
}

/** \brief Put a component data in a component container
 *
 * T must match the component data type or can be implicitly cast to it.
 *
 * \param comp the data
 * \return std::shared_ptr<Container> the container
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<Container> Create(T&& comp) {
    return std::static_pointer_cast<Container>(std::allocate_shared<ContainerObject<C,T>>(TrillekAllocator<ContainerObject<C,T>>(), std::forward<T>(comp)));
}

/** \brief Put a component data in a component container
 *
 * This version returns a const Container.
 *
 * T must match the component data type or can be implicitly cast to it.
 *
 * \param comp the data
 * \return std::shared_ptr<const Container> the container
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<const Container> CreateConst(T&& comp) {
    return std::static_pointer_cast<const Container>(std::allocate_shared<ContainerObject<C,T>>(TrillekAllocator<ContainerObject<C,T>>(), std::forward<T>(comp)));
}

} // namespace component
} // namespace trillek

#endif
