#ifndef LUA_HPP_INCLUDED
#define LUA_HPP_INCLUDED

#include <string>

#include "trillek.hpp"
#include "systems/system-base.hpp"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace trillek {
namespace script {

typedef int (LuaRegisterFunction)(lua_State*);

class LuaSystem : public SystemBase {
public:
    LuaSystem();
    ~LuaSystem();
    /**
     * \brief Starts the Lua scripting system.
     *
     */
    void Start();

    void ThreadInit() override { }

    /**
     * \brief Causes an update in the system based on the change in time.
     *
     * Updates the state of the system based off how much time has elapsed since the last update.
     */
    void RunBatch() const override { };

    /**
     * \brief Adds a Shape component to the system.
     *
     * A dynamic_pointer_case is applied to the component shared_ptr to cast it to
     * a Shape component. If the cast results in a nullptr the method returns
     * without adding the Shape component.
     * \param const unsigned int entity_id The entity ID the compoennt belongs to.
     * \param std::shared_ptr<ComponentBase> component The component to add.
     */
    void AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component);

    /**
    * \brief Calls a systems's register function with this systems lua_State.
    *
    * \param LuaRegisterFunction The function to call with this system's lua_State.
    */
    void RegisterSystem(LuaRegisterFunction func);

    /**
    * \brief Load Lua script from a file.
    *
    * \param const std::string fname The name of the file to load.
    */
    bool LoadFile(const std::string fname);

    /** \brief Handle incoming events to update data
     *
     * This function is called once every frame. It is the only
     * function that can write data. This function is in the critical
     * path, job done here must be simple.
     *
     * If event handling need some batch processing, a task list must be
     * prepared and stored temporarily to be retrieved by RunBatch().
     */
    void HandleEvents(const frame_tp& timepoint) override;

    /** \brief Save the data and terminate the system
     *
     * This function is called when the program is closing
     */
    void Terminate() override;

private:
    lua_State* L;
    frame_unit delta; // The time since the last HandleEvents was called.
};

} // End of script
} // End of trillek
#endif
