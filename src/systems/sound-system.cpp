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

std::shared_ptr<Sound> System::GetSound(const std::string& id) {
    if(sounds.find(id) != sounds.end()) {
        std::shared_ptr<Sound> sound(new Sound);
        alGenSources(1, &sound->src);

        if(alGetError() != AL_NO_ERROR) {
            std::cout << "Failed to create OpenAL source for sound: " << id << std::endl;
            return nullptr;
        }

        // create openal buffer from source
        sound->buff = alureCreateBufferFromFile(sounds[id]->src.c_str());

        if(sound->buff  == 0) {
            std::cout << "Could not load: " << sound->buff << " : " << alureGetErrorString() << std::endl;
            return nullptr;
        }

        alSourcei(sound->src, AL_BUFFER, sound->buff);
        alSourcei(sound->src, AL_LOOPING, sounds[id]->loop);
        alSourcef(sound->src, AL_GAIN, sounds[id]->volume);
        alSourcei(sound->src, AL_SOURCE_RELATIVE, !sounds[id]->spatial);

        return sound;
    }
    return nullptr;
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

bool System::Serialize(rapidjson::Document& document) {

}

bool System::DeSerialize(rapidjson::Value& node) {
    if(node.IsArray()) {

        for(auto sound_itr = node.Begin(); sound_itr != node.End(); sound_itr ++) {

            if(sound_itr->IsObject()) {
                auto& element = (*sound_itr);

                std::shared_ptr<sound_info> sinfo = std::shared_ptr<sound_info>(new sound_info());

                if(element.HasMember("id") && element["id"].IsString()) {
                    sinfo->id = element["id"].GetString();
                }

                if(element.HasMember("src") && element["src"].IsString()) {
                    sinfo->src = element["src"].GetString();
                }

                if(element.HasMember("loop") && element["loop"].IsBool()) {
                    sinfo->loop = element["loop"].GetBool();
                }

                if(element.HasMember("volume") && element["volume"].IsNumber()) {
                    sinfo->volume = element["volume"].GetDouble();
                }

                if(element.HasMember("spatial") && element["spatial"].IsBool()) {
                    sinfo->spatial = element["spatial"].GetBool();
                }

                // store sound info
                sounds[sinfo->id] = sinfo;

            }

        }

    }
}

} // end of namespace sound
} // end of namespace trillek



