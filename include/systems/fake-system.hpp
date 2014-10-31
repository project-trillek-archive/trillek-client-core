#ifndef FAKESYSTEM_H_INCLUDED
#define FAKESYSTEM_H_INCLUDED

#include "systems/system-base.hpp"
#include "logging.hpp"

namespace trillek {

class FakeSystem : public SystemBase {

public:

    FakeSystem() {};
    virtual ~FakeSystem() {};

    void HandleEvents(frame_tp timepoint) override {
        LOGMSGC(DEBUG) << "handle events of frame " << timepoint;
    };

    void RunBatch() const override {
        LOGMSGC(DEBUG) << "run batch";
    };

    void Terminate() override {
        LOGMSGC(DEBUG) << "terminating";
    };
};
}

#endif // FAKESYSTEM_H_INCLUDED
