#ifndef NETWORKNODE_H_INCLUDED
#define NETWORKNODE_H_INCLUDED

#include "map-array.hpp"

namespace trillek {
class NetworkNode {
public:
    NetworkNode() {};
    virtual ~NetworkNode() {};

    static void AddEntity(const id_t id, int&& cnx) {
        connection_map[id] = std::move(cnx);
    }

    static void RemoveEntity(const id_t id) {
        connection_map.clear(id);
    }
private:
    static MapArray<int> connection_map;
};
}


#endif // NETWORKNODE_H_INCLUDED
