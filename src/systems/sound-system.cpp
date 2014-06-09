#include "systems/sound-system.hpp"

namespace trillek {
namespace sound {

Sound::Sound() : src(0), buff(0) {

}

Sound::~Sound() {
    if(src) {
        alDeleteSources(1, &src);
    }

    if(buff) {
        alDeleteBuffers(1, &buff);
    }
}

void Sound::Play() {
    auto eos_callback = [](void* userdata, ALuint source) {
        std::cout << "Source has stopped." << std::endl;
    };

    if(alurePlaySource(src, eos_callback, NULL) == AL_FALSE) {
        std::cout << "Failed to start source." << std::endl;
    }
}

void Sound::Pause() {
    if(alurePauseSource(src) == AL_FALSE) {
        std::cout << "Failed to pause source." << std::endl;
    }
}

void Sound::Resume() {
    if(alureResumeSource(src) == AL_FALSE) {
        std::cout << "Failed to resume source." << std::endl;
    }
}

void Sound::Stop() {
    if(alureStopSource(src, AL_TRUE) == AL_FALSE) {
        std::cout << "Failed to stop source." << std::endl;
    }
}

void Sound::SetPosition(glm::vec3 position) {
    alSource3f(src, AL_POSITION, position.x, position.y, position.z);
}

void Sound::SetVelocity(glm::vec3 velocity) {
    alSource3f(src, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Sound::SetDirection(glm::vec3 direction) {
    alSource3f(src, AL_DIRECTION, direction.x, direction.y, direction.z);
}

void Sound::SetOuterConeAngle(float angle) {
    alSourcef(src, AL_CONE_OUTER_ANGLE, angle);
}

std::once_flag System::only_one;
std::shared_ptr<System> System::instance = nullptr;

System::~System() {
    alureShutdownDevice();
}

std::shared_ptr<Sound> System::CreateSoundFromFile(const std::string& file_name) {
    std::shared_ptr<Sound> sound(new Sound);

    alGenSources(1, &sound->src);

    if(alGetError() != AL_NO_ERROR) {
        std::cout << "Failed to create OpenAL source." << std::endl;
        return 0;
    }

    sound->buff = alureCreateBufferFromFile(file_name.c_str());

    if(sound->buff == 0) {
        std::cout << "Could not load: " << file_name << " : " << alureGetErrorString() << std::endl;
        alDeleteSources(1, &sound->src);
        return 0;
    }

    alSourcei(sound->src, AL_BUFFER, sound->buff);

    return sound;
}

void System::Update() {
    alureUpdate();
}

void System::SetListenerPosition(glm::vec3 position) {
    alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void System::SetListenerVelocity(glm::vec3 velocity) {
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void System::SetListenerOrientation(glm::vec3 at, glm::vec3 up) {
    ALfloat orientation[] = {at.x, at.y, at.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orientation);
}


} // end of namespace sound
} // end of namespace trillek



