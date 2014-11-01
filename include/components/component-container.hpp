#ifndef COMPONENT_CONTAINER_HPP_INCLUDED
#define COMPONENT_CONTAINER_HPP_INCLUDED

#include "trillek.hpp"
#include "type-id.hpp"

namespace trillek {
namespace component {

class Container {
public:
    typedef std::underlying_type<Component>::type component_type;

    template<Component C>
    bool Is() { return static_cast<component_type>(C) == type; };

protected:
    Container(component_type n) : type(n) {};

private:
    component_type type;
};

/**
 * \brief A class to contain an arbitrary value.
 *
 * This class is used to carry dynamic typed values.
 * The type contained can be determined with GetType() or Is<T>()
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

/** \brief Create an alias to a shared pointer
 *
 * \param ct const std::shared_ptr<Container>& the original pointer
 * \return std::shared_ptr<T> the alias
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<T> Get(const std::shared_ptr<Container>& ct) {
    return std::shared_ptr<T>(ct,&std::static_pointer_cast<ContainerObject<C>>(ct)->Get());
}

/** \brief Create an alias to a shared pointer
 *
 * \param ct const std::shared_ptr<Container>& the original pointer
 * \return std::shared_ptr<T> the alias
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<const T> Get(const std::shared_ptr<const Container>& ct) {
    return std::shared_ptr<const T>(ct,&std::static_pointer_cast<const ContainerObject<C>>(ct)->Get());
}

/** \brief Create an alias to a shared pointer
 *
 * \param ct const std::shared_ptr<Container>& the original pointer
 * \return std::shared_ptr<T> the alias
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<Container> Create(const typename type_trait<C>::value_type& comp) {
    return std::static_pointer_cast<Container>(std::allocate_shared<ContainerObject<C,T>>(TrillekAllocator<ContainerObject<C,T>>(), comp));
}

/** \brief Create an alias to a shared pointer
 *
 * \param ct const std::shared_ptr<Container>& the original pointer
 * \return std::shared_ptr<T> the alias
 *
 */
template<Component C, class T=typename type_trait<C>::value_type>
std::shared_ptr<const Container> CreateConst(const typename type_trait<C>::value_type& comp) {
    return std::static_pointer_cast<const Container>(std::allocate_shared<ContainerObject<C,T>>(TrillekAllocator<ContainerObject<C,T>>(), comp));
}

} // namespace component
} // namespace trillek

#endif
