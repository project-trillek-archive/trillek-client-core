#ifndef OS_EVENT_HPP_INCLUDED
#define OS_EVENT_HPP_INCLUDED

#include "trillek.hpp"

namespace trillek {

struct KeyboardEvent {
    enum KEY_ACTION { KEY_DOWN, KEY_UP, KEY_REPEAT, KEY_CHAR };
    int key;
    int scancode;
    KEY_ACTION action;
    int mods;
};


} // namespace trillek

#endif
