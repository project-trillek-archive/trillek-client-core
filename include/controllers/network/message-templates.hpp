#ifndef MESSAGETEMPLATES_H_INCLUDED
#define MESSAGETEMPLATES_H_INCLUDED

#include "controllers/network/message.hpp"

namespace trillek { namespace network {
extern template	void Message::Resize<CLIENT>(size_t new_size);
extern template	void Message::Resize<SERVER>(size_t new_size);
extern template	void Message::Resize<NONE>(size_t new_size);
}
}

#endif // MESSAGETEMPLATES_H_INCLUDED
