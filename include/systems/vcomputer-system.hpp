#ifndef CPU_HPP_INCLUDED
#define CPU_HPP_INCLUDED

#include "trillek-scheduler.hpp"
#include "system-base.hpp"
#include "VComputer.hpp"
#include "devices/GKeyb.hpp"
#include <memory>
#include <fstream>
#include "systems/dispatcher.hpp"
#include "os-event.hpp"
#include "resources/pixel-buffer.hpp"
#include "devices/TDA.hpp"

namespace trillek {

class ComponentBase;

using namespace vm;

enum CPU_TYPE { TR3200, DCPU, DCPUN };

// Simple pair of TDA screen to pixelbuffer.
typedef std::pair<std::shared_ptr<resource::PixelBuffer>, std::shared_ptr<dev::tda::TDADev>> ScreenImage;

// Struct to hold various CPU specific items together.
struct Computer {
    Computer() : rom(new byte_t[32 * 1024]), rom_size(0) { }
    ~Computer() {
        if (this->rom) {
            this->vc->Off();
            delete this->rom;
        }
    }
    byte_t *rom;
    size_t rom_size;
    std::unique_ptr<VComputer> vc;
    std::list<std::shared_ptr<IDevice>> devices;
};

class VComputerSystem : public SystemBase,
    public event::Subscriber<KeyboardEvent> {
public:

    VComputerSystem();
    ~VComputerSystem();

    /** \brief Add a computer/CPU to the system.
     *
     * \param const id_t entityID The entity ID the computer/CPU belongs to.
     * \param CPU_TYPE The type of CPU to "install" in the computer.
    */
    void AddComputer(const id_t entity_id, CPU_TYPE type);

    /** \brief Removes a computer/CPU from the system.
    *
    * \param const id_t entityID The entity ID the computer/CPU belongs to.
    */
    void RemoveComputer(const id_t entity_id);

    /** \brief Sets the specified device for the entity ID to device.
     *
     * \param const id_t entityID The entity ID the computer/CPU belongs to.
     * \param const unsigned int The slot to assign device to.
     * \param std::shared_ptr<IDevice> device The device to install.
     */
    void SetDevice(const id_t entity_id, const unsigned int slot, std::shared_ptr<IDevice> device);

    /** \brief Remove a device from the specified slot.
     *
     * \param const id_t entityID The entity ID the computer/CPU belongs to.
     * \param const unsigned int The slot to remove the device from.
     */
    void RemoveDevice(const id_t entity_id, const unsigned int slot);


    /** \brief Load a ROM file from disk for the specified computer.
     *
     * \param const id_t entityID The entity ID the computer belongs to.
     * \param const std::string fname The name of the ROM file to load.
     * \return bool Wether or not the file loaded successfully (also returns false if no computer exists for the given entity_id).
     */
    bool LoadROMFile(const id_t entity_id, std::string fname);

    /** \brief Turns specified the computer on.
     *
     * \param const id_t entityID The entity ID the computer belongs to.
     */
    void TurnComptuerOn(const id_t entity_id);

    /** \brief Turns specified the computer off.
     *
     * \param const id_t entityID The entity ID the computer belongs to.
     */
    void TurnComptuerOff(const id_t entity_id);

    /** \brief This function is executed when a thread is attached to the system
     */
    virtual void ThreadInit() {};

    /** \brief Handle incoming events to update data
     *
     * This function is called once every frame. It is the only
     * function that can write data. This function is in the critical
     * path, job done here must be simple.
     *
     * If event handling need some batch processing, a task list must be
     * prepared and stored temporarily to be retrieved by RunBatch().
     */
    virtual void HandleEvents(frame_tp timepoint);

    /** \brief Make all pre-update or post-update work.
     *
     * Pre-update work is prepared by HandleEvents() and executed here.
     * Updates must be done through events to be taken into account at
     * next frame.
     * Pre-update work is needed when data retrieved from event must be
     * processed (i.e make computation, reassemble, etc.) before being stored.
     *
     * Post-update work is the transformation of data to render entities or
     * other operations.
     *
     * Note that this function is const, i.e the system is in read-only mode.
     */
    virtual void RunBatch() const { }

    /** \brief Save the data and terminate the system
     *
     * This function is called when the program is closing
     */
    virtual void Terminate() { }

    /**
     * \brief Adds a component to the system.
     *
     * \param const id_t entityID The entity ID the compoennt belongs to.
     * \param std::shared_ptr<ComponentBase> component The component to add.
     */
    virtual void AddComponent(const id_t entity_id, std::shared_ptr<ComponentBase> component);

    void Notify(const KeyboardEvent* key_event);
private:

    //VComputer vc;
    //byte_t *rom;
    //size_t rom_size;
    frame_unit delta; // The time since the last HandleEvents was called.

    std::shared_ptr<resource::PixelBuffer> pBuffer;
    std::shared_ptr<dev::gkeyboard::GKeyboardDev> gkeyb;
    std::map<id_t, ScreenImage> pixelBuffers;
    std::map<id_t, Computer> computers;
};

} // namespace trillek

#endif // CPU_HPP_INCLUDED
