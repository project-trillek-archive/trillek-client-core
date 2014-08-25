#include <memory>
#include <vector>
#include <mutex>
#include "controllers/network/authentication-handler.hpp"
#include "controllers/network/connection-data.hpp"

namespace trillek { namespace network {
const std::vector<unsigned char> ConnectionData::_states{ AUTH_NONE, AUTH_INIT, AUTH_KEY_EXCHANGE, AUTH_SHARE_KEY };
}
}