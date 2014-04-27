#ifndef FAKESYSTEM_H_INCLUDED
#define FAKESYSTEM_H_INCLUDED

#include "System.h"
#include <iostream>

namespace trillek {

    class FakeSystem : public System {

    public:

        FakeSystem() {};
        virtual ~FakeSystem() {};

        void HandleEvents(const std::chrono::time_point<std::chrono::steady_clock, frame_unit>& timepoint) override {
            std::cout << "handle events" << std::endl;
        };

        void RunBatch() const override {
            std::cout << "run batch" << std::endl;
        };
    };
}

#endif // FAKESYSTEM_H_INCLUDED
