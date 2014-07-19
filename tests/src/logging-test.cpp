#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include "logging.hpp"
#include <iostream>

class Logging_Messages_Test;

namespace trillek {
namespace reflection {
TRILLEK_MAKE_IDTYPE(Logging_Messages_Test, ((~0)-1))
}
}
TEST(Logging, Messages) {
    LOGMSG(ERROR) << "words about something bad";
    LOGMSGC(ERROR) << "such class";
    LOGMSGFOR(NOTICE, Logging_Messages_Test) << "some text";
    LOGMSGC(INFO) << "info class" << '\n';
}
