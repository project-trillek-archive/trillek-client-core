#ifndef TRANSFORMSYSTEM_HPP_INCLUDED
#define TRANSFORMSYSTEM_HPP_INCLUDED

#include <memory>
#include <map>
#include <mutex>

#include "trillek.hpp"
#include "util/json-parser.hpp"
#include "systems/rewindable-map.hpp"
#include "atomic-map.hpp"

namespace trillek {

class Transform;
namespace physics {
class PhysicsSystem;
}

// Stores a mapping of entity ID to transform that can
// be accessed via static methods anywhere.
class TransformMap : public util::Parser {
private:
    TransformMap() : Parser("transforms") { }
    TransformMap(const TransformMap& right) : Parser("transforms") {
        instance = right.instance;
    }
    TransformMap& operator=(const TransformMap& right) {
        if (this != &right) {
            instance = right.instance;
        }

        return *this;
    }
    static std::once_flag only_one;
    static std::shared_ptr<TransformMap> instance;
public:
    static std::shared_ptr<TransformMap> GetInstance() {
        std::call_once(TransformMap::only_one,
            [ ] () {
            TransformMap::instance.reset(new TransformMap());
        }
        );

        return TransformMap::instance;
    }
    ~TransformMap() { }

    // Inherited from Parse
    virtual bool Serialize(rapidjson::Document& document);

    // Inherited from Parse
    virtual bool Parse(rapidjson::Value& node);
};

} // End of trillek

#endif
