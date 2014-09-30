#include "systems/lua-system.hpp"
#include <iostream>

namespace trillek {
namespace script {

int luaopen_Transform(lua_State*);
int luaopen_LuaSys(lua_State*);

LuaSystem::LuaSystem() {
    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
    this->event_handlers[reflection::GetTypeID<KeyboardEvent>()];
    event::Dispatcher<MouseBtnEvent>::GetInstance()->Subscribe(this);
    this->event_handlers[reflection::GetTypeID<MouseBtnEvent>()];
    event::Dispatcher<MouseMoveEvent>::GetInstance()->Subscribe(this);
    this->event_handlers[reflection::GetTypeID<MouseMoveEvent>()];
}
LuaSystem::~LuaSystem() { }

void LuaSystem::Start() {
    this->L = luaL_newstate();
    luaL_openlibs(L);
    RegisterTypes();
}

void LuaSystem::AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component) { }

void LuaSystem::RegisterSystem(LuaRegisterFunction func) {
    if (this->L) {
        func(this->L);
    }
}

bool LuaSystem::LoadFile(const std::string fname) {
    if (!this->L) {
        return false;
    }
    if (luaL_dofile(L, fname.c_str())) {
        // TODO: Ommit error string about the filename that failed to load.
        std::cerr << lua_tostring(L, -1) << std::endl;
        return false;
    }
    return true;
}

void LuaSystem::HandleEvents(const frame_tp& timepoint) {
    static frame_tp last_tp;
    this->delta = timepoint - last_tp;
    last_tp = timepoint;
    // Call the Update method from Lua. A time delta is passed in.
    if (this->L) {
        //lua_getglobal(L, "Update");
        //lua_pushnumber(L, delta.count() * 1.0E-9);
        //lua_pcall(L, 1, 0, 0);
    }
}

void LuaSystem::Terminate() {
    lua_close(L);
}

void LuaSystem::Notify(const KeyboardEvent* key_event) {
    if (this->event_handlers.find(reflection::GetTypeID<KeyboardEvent>()) != this->event_handlers.end()) {
        for (auto handler : this->event_handlers[reflection::GetTypeID<KeyboardEvent>()]) {
            lua_getglobal(L, handler.c_str());
            if (key_event->action == KeyboardEvent::KEY_DOWN) {
                lua_pushstring(L, "Down");
            }
            else if (key_event->action == KeyboardEvent::KEY_UP) {
                lua_pushstring(L, "Up");
            }
            else if (key_event->action == KeyboardEvent::KEY_REPEAT) {
                lua_pushstring(L, "Repeat");
            }
            lua_pushnumber(L, key_event->key);
            lua_pcall(L, 2, 0, 0);
        }
    }
}

void LuaSystem::Notify(const MouseBtnEvent* mousebtn_event) {
    if (this->event_handlers.find(reflection::GetTypeID<MouseBtnEvent>()) != this->event_handlers.end()) {
        for (auto handler : this->event_handlers[reflection::GetTypeID<MouseBtnEvent>()]) {
            lua_getglobal(L, handler.c_str());
            if (mousebtn_event->action == MouseBtnEvent::DOWN) {
                lua_pushstring(L, "Down");
            }
            else if (mousebtn_event->action == MouseBtnEvent::UP) {
                lua_pushstring(L, "Up");
            }

            if (mousebtn_event->button == MouseBtnEvent::LEFT) {
                lua_pushstring(L, "Left");
            }
            else if (mousebtn_event->button == MouseBtnEvent::RIGHT) {
                lua_pushstring(L, "Right");
            }
            else if (mousebtn_event->button == MouseBtnEvent::MIDDLE) {
                lua_pushstring(L, "Middle");
            }
            lua_pcall(L, 2, 0, 0);
        }
    }
}

void LuaSystem::Notify(const MouseMoveEvent* mousemove_event) {
    if (this->event_handlers.find(reflection::GetTypeID<MouseMoveEvent>()) != this->event_handlers.end()) {
        for (auto handler : this->event_handlers[reflection::GetTypeID<MouseMoveEvent>()]) {
            lua_getglobal(L, handler.c_str());
            lua_pushnumber(L, mousemove_event->new_x);
            lua_pushnumber(L, mousemove_event->new_y);
            lua_pushnumber(L, mousemove_event->old_x);
            lua_pushnumber(L, mousemove_event->old_y);
            lua_pushnumber(L, mousemove_event->norm_x);
            lua_pushnumber(L, mousemove_event->norm_y);
            lua_pcall(L, 6, 0, 0);
        }
    }
}

} // End of script
} // End of trillek
