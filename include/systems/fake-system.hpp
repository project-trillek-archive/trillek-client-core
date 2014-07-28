#ifndef FAKESYSTEM_H_INCLUDED
#define FAKESYSTEM_H_INCLUDED

#include "systems/system-base.hpp"
#include <iostream>

namespace trillek {

class FakeSystem : public SystemBase {

public:

    FakeSystem() {};
    virtual ~FakeSystem() {};

    void HandleEvents(const frame_tp& timepoint) override {
        std::cout << "handle events of frame " << timepoint.time_since_epoch().count() << std::endl;
    };

    void RunBatch() const override {
        std::cout << "run batch" << std::endl;
    };

    void Terminate() override {
        std::cout << "terminating" << std::endl;
    };
};
}

#endif // FAKESYSTEM_H_INCLUDED
