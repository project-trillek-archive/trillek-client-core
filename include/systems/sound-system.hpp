#ifndef SOUND_SYSTEM_HPP_INCLUDED
#define SOUND_SYSTEM_HPP_INCLUDED

#include <memory>
#include <string>
#include <mutex>
#if __APPLE__
#include "OpenAL/alure.h"
#else
#include "AL/alure.h"
#endif
#include "glm/glm.hpp"
#include <iostream>
#include <unordered_map>
#include "transform.hpp"
#include "type-id.hpp"
#include "util/json-parser.hpp"
#include "systems/system-base.hpp"
#include "systems/dispatcher.hpp"

namespace trillek {
namespace sound {

class Sound {
public:
    Sound();
    ~Sound();

    /** \brief Start Sound
     */
    void Play();

    /** \brief Pause Sound
     */
    void Pause();

    /** \brief Resumes Sound
     */
    void Resume();

    /** \brief Stops Sound
     */
    void Stop();

    /** \brief Set sound position
     *
     * \param position glm::vec3
     */
    void SetPosition(glm::vec3 position);

    /** \brief Set sound velocity
     *
     * \param velocity glm::vec3
     */
    void SetVelocity(glm::vec3 velocity);

    /** \brief Set sound direction
     *
     * \param direction glm::vec3
     */
    void SetDirection(glm::vec3 direction);

    /** \brief outer angle of sound cone
     *
     * \param angle float
     */
    void SetOuterConeAngle(float angle);

    ALuint src, buff; /// OpenAL source and buffer object
}; // end of class Sound


class System : public util::Parser, public SystemBase {
private:
    System() : Parser("sounds"), last_transform_frame(-1) { }
    System(const System& right) : Parser("sounds")  {
        instance = right.instance;
    }
    System& operator=(const System& right) {
        if(this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<System> instance;
public:
    static std::shared_ptr<System> GetInstance();

    ~System();

    // inherited from system-base
    virtual void HandleEvents(frame_tp timepoint);
    virtual void RunBatch() const;
    virtual void Terminate();

    /** \brief Creates and returns a Sound if successful
     *
     * \param id const std::string&
     * \return std::shared_ptr<Sound>
     */
    std::shared_ptr<Sound> GetSound(const std::string& id);

    /** \brief set the position manualy if no transform is set in the system
     *
     * \param position const glm::vec3&
     */
    void SetListenerPosition(const glm::vec3& position);

    /** \brief Sets the velocity of the sound listener
     *
     * \param velocity glm::vec3
     */
    void SetListenerVelocity(const glm::vec3& velocity);

    /** \brief Sets the orientation of the sound listener
     *
     * \param at glm::vec3
     * \param up glm::vec3
     */
    void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up);

    // Inherited from SerializeBase
    virtual bool Serialize(rapidjson::Document& document);

    // Inherited from SerializeBase
    virtual bool Parse(rapidjson::Value& node);

private:
    struct sound_info {
        sound_info() : id("") , src(""), loop(false), spatial(false), volume(1.0) {};
        std::string id, src;
        bool loop, spatial;
        double volume;
    };

    frame_tp last_transform_frame;

    std::unordered_map<std::string, std::shared_ptr<sound_info>> sounds;
}; // end of class System

} // end of namespace sound

namespace reflection {
TRILLEK_MAKE_IDTYPE(sound::Sound, 801)
TRILLEK_MAKE_IDTYPE(sound::System, 800)
} // namespace reflection

} // end of namespace trillek

#endif // SOUND_SYSTEM_HPP_INCLUDED

