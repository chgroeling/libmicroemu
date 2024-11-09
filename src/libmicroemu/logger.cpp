
#include "libmicroemu/logger.h"
using namespace libmicroemu;

void (*StaticLogger::callback_)(libmicroemu::LogLevel level, const char *, ...) noexcept;
