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

struct MouseBtnEvent {
    enum MOUSE_BTN_ACTION { DOWN, UP };
    enum MOUSE_BTN { LEFT, RIGHT, MIDDLE };
    MOUSE_BTN_ACTION action;
    MOUSE_BTN button;
};

struct MouseMoveEvent {
    double norm_x, norm_y; // Resolution independent new x, y (0-1) from upper-left to lower-right.
    int old_x, old_y; // Client space old x, y.
    int new_x, new_y; // Client space new x, y.
};

} // namespace trillek

#endif
