#ifndef OS_EVENT_HPP_INCLUDED
#define OS_EVENT_HPP_INCLUDED

#include "trillek.hpp"
#include "type-id.hpp"

namespace trillek {

struct KeyboardEvent {
    enum KEY_ACTION { KEY_DOWN, KEY_UP, KEY_REPEAT, KEY_CHAR };
    int key;
    int scancode;
    KEY_ACTION action;
    int mods;
};

namespace reflection {

TRILLEK_MAKE_IDTYPE_NAME(KeyboardEvent, "KeyboardEvent", 5000);

} // namespace reflection
} // namespace trillek

#endif
