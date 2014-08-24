#ifndef NETWORKCONTROLLERTEMPLATES_H_INCLUDED
#define NETWORKCONTROLLERTEMPLATES_H_INCLUDED

namespace trillek { namespace network {
extern template void NetworkController::CloseConnection<CLIENT>(const Message* frame) const;
extern template	void NetworkController::CloseConnection<SERVER>(const Message* frame) const;
}
}

#endif // NETWORKCONTROLLERTEMPLATES_H_INCLUDED
