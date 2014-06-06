#include "systems/sound-system.hpp"
#include <iostream>

namespace trillek {
    namespace sound {
        ///////////////////
        //// SOUND /////
        //////////////////

        Sound::Sound() : src( 0 ), buff( 0 ) {

        }

        Sound::~Sound() {
            if( src ) {
                alDeleteSources( 1, &src );
            }

            if( buff ) {
                alDeleteBuffers( 1, &buff );
            }
        }

        void Sound::Play() {
            auto eos_callback = []( void* userdata, ALuint source ) {
                std::cout << "Source has stopped." << std::endl;
            };

            if( alurePlaySource( src, eos_callback, NULL ) == AL_FALSE ) {
                std::cout << "Failed to start source." << std::endl;
            }
        }

        void Sound::Pause() {
            if( alurePauseSource( src ) == AL_FALSE ) {
                std::cout << "Failed to pause source." << std::endl;
            }
        }

        void Sound::Resume() {
            if( alureResumeSource( src ) == AL_FALSE ) {
                std::cout << "Failed to resume source." << std::endl;
            }
        }

        void Sound::Stop() {
            if( alureStopSource( src, AL_TRUE ) == AL_FALSE ) {
                std::cout << "Failed to stop source." << std::endl;
            }
        }

        ///////////////////
        //// SYSTEM ////
        //////////////////

        std::shared_ptr<System> System::instance = nullptr;

        System::System() {
            if( alureInitDevice( NULL, NULL ) == false ) {
                std::cout << "Failed to open OpenAL device: " << alureGetErrorString() << std::endl;
            }
        }

        System::~System() {
            alureShutdownDevice();
        }

        std::shared_ptr<System> System::GetInstance() {
            if( System::instance == nullptr ) {
                System::instance.reset( new System );
            }

            return System::instance;
        }

        std::shared_ptr<Sound> System::CreateSoundFromFile( const std::string& file_name ) {
            std::shared_ptr<Sound> sound( new Sound );

            alGenSources( 1, &sound->src );

            if( alGetError() != AL_NO_ERROR ) {
                std::cout << "Failed to create OpenAL source." << std::endl;
                return 0;
            }

            sound->buff = alureCreateBufferFromFile( file_name.c_str() );

            if( sound->buff == 0 ) {
                std::cout << "Could not load: " << file_name << " : " << alureGetErrorString() << std::endl;
                alDeleteSources( 1, &sound->src );
                return 0;
            }

            alSourcei( sound->src, AL_BUFFER, sound->buff );

            return sound;
        }

        void System::Update() {
            alureUpdate();
        }

    }
}


