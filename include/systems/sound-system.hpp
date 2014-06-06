#ifndef SOUND_SYSTEM_HPP_INCLUDED
#define SOUND_SYSTEM_HPP_INCLUDED

#include "AL/alure.h"
#include <memory>
#include <string>

namespace trillek {
    namespace sound {
        class Sound {
        public:
            Sound();
            ~Sound();

            /** \brief Start playing the Sound
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

            /** \brief Resumes a paused Sound
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

            ALuint src, buff; /// OpenAL source and buffer object
        };

        class System {
        public:
            System();
            ~System();

            /** \brief Creates a sound object from a file
             *
             * \param file_name const std::string&
             * \return std::shared_ptr<Sound>
             *
             */
            std::shared_ptr<Sound> CreateSoundFromFile( const std::string& file_name );

            void Update();

            static std::shared_ptr<System> GetInstance();

        private:
            static std::shared_ptr<System> instance;

        };
    } // sound
} // trillek

#endif // SOUND_SYSTEM_HPP_INCLUDED
