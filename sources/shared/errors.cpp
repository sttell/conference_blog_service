#include "errors.h"

#define IMPLEMENT_DEFAULT_CONSTRUCTORS(classname, base_class) \
    classname::classname(const std::string& description) noexcept : base_class(description) {} \
     classname::classname(const char* description) noexcept : base_class(description) {}

namespace exceptions {

    IMPLEMENT_DEFAULT_CONSTRUCTORS(WrongPathError,            std::runtime_error)
    IMPLEMENT_DEFAULT_CONSTRUCTORS(UnexpectedFileFormatError, WrongPathError)
    IMPLEMENT_DEFAULT_CONSTRUCTORS(FileNotFoundError,         WrongPathError)
    IMPLEMENT_DEFAULT_CONSTRUCTORS(ConfigNotFoundError,       FileNotFoundError)


} // namespace exceptions