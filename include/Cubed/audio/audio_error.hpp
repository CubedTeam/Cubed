#pragma once
#include "Cubed/tools/log.hpp"

#include <AL/al.h>
namespace Cubed {
inline void check_al_error(
    std::source_location loaction = std::source_location::current()) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        Logger::error("File {} Line {} Function {} OpenAL Error {} ",
                      loaction.file_name(), loaction.line(),
                      loaction.function_name(), alGetString(error));
    }
}
} // namespace Cubed
