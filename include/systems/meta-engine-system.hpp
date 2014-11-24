#ifndef METAENGINESYSTEM_HPP_INCLUDED
#define METAENGINESYSTEM_HPP_INCLUDED

#include "systems/system-base.hpp"

namespace trillek {
class MetaEngineSystem final : public SystemBase {
    void ThreadInit() override;

    void RunBatch() const override;

    void HandleEvents(frame_tp timepoint) override;

    void Terminate() override;
};
}

#endif // METAENGINESYSTEM_HPP_INCLUDED
