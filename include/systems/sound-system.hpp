#ifndef SOUND_SYSTEM_HPP_INCLUDED
#define SOUND_SYSTEM_HPP_INCLUDED

#include <memory>
#include <string>
#include <mutex>
#include "AL/alure.h"
#include "glm/glm.hpp"
#include <iostream>

namespace trillek {
namespace sound {

class Sound {
public:
    Sound();
    ~Sound();

    /** \brief Start Sound
     *
     * \return void
     *
     */
    void Play();

    /** \brief Pause Sound
     *
     * \return void
     *
     */
    void Pause();

    /** \brief Resumes Sound
     *
     * \return void
     *
     */
    void Resume();

    /** \brief Stops Sound
     *
     * \return void
     *
     */
    void Stop();

    /** \brief Set sound position
     *
     * \param position glm::vec3
     * \return void
     *
     */
    void SetPosition(glm::vec3 position);

    /** \brief Set sound velocity
     *
     * \param velocity glm::vec3
     * \return void
     *
     */
    void SetVelocity(glm::vec3 velocity);

    /** \brief Set sound direction
     *
     * \param direction glm::vec3
     * \return void
     *
     */
    void SetDirection(glm::vec3 direction);

    /** \brief outer angle of sound cone
     *
     * \param angle float
     * \return void
     *
     */
    void SetOuterConeAngle(float angle);

    ALuint src, buff; /// OpenAL source and buffer object
}; // end of class Sound

class System {
private:
    System() { }
    System(const System& right) {
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
    static std::shared_ptr<System> GetInstance() {
        std::call_once(System::only_one,
        [ ]() {
            System::instance.reset(new System());

            if(alureInitDevice(NULL, NULL) == false) {
                std::cout << "Failed to open OpenAL device: " << alureGetErrorString() << std::endl;
            }

        });

        return System::instance;
    }

    ~System();

    /** \brief Creates a sound object from a file
     *
     * \param file_name const std::string&
     * \return std::shared_ptr<Sound>
     *
     */
    std::shared_ptr<Sound> CreateSoundFromFile(const std::string& file_name);

    void Update();

    /** \brief Sets the position of sound listener
     *
     * \param position glm::vec3
     * \return void
     *
     */
    void SetListenerPosition(glm::vec3 position);

    /** \brief Sets the velocity of the sound listener
     *
     * \param velocity glm::vec3
     * \return void
     *
     */
    void SetListenerVelocity(glm::vec3 velocity);

    /** \brief Sets the orientation of the sound listener
     *
     * \param at glm::vec3
     * \param up glm::vec3
     * \return void
     *
     */
    void SetListenerOrientation(glm::vec3 at, glm::vec3 up);
}; // end of class System

} // end of namespace sound
} // end of namespace trillek

#endif // SOUND_SYSTEM_HPP_INCLUDED

