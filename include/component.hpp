#ifndef COMPONENT_HPP_INCLUDED
#define COMPONENT_HPP_INCLUDED

#include <vector>

namespace trillek {

class Property;

class ComponentBase {
public:
    ComponentBase() : component_type_id(0) { }
    ~ComponentBase() { }

    /**
     * \brief Returns a component with the specified name.
     *
     * \param[in] const std::vector<Property> &properties The creation properties for the component.
     * \return bool True if initialization finished with no errors.
     */
    virtual bool Initialize(const std::vector<Property> &properties) = 0;

    unsigned int component_type_id;
};

} // namespace trillek

#endif
