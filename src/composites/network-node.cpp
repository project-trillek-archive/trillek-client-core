#include "composites/network-node.hpp"

namespace trillek {
MapArray<TCPConnection> NetworkNode::connection_map;
MapArray<network::socket_t> NetworkNode::fd_map;
}
