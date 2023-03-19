#ifndef SERVER_ERRORS_H
#define SERVER_ERRORS_H

#include <stdexcept>
#include <string>

#define REGISTER_EXCEPTION_TYPE(classname, base_class)               \
    class classname : public base_class {                            \
    public:                                                          \
        explicit classname(const std::string& description) noexcept; \
        explicit classname(const char* description) noexcept;        \
    }

namespace exceptions {

    REGISTER_EXCEPTION_TYPE(WrongPathError,            std::runtime_error);
    REGISTER_EXCEPTION_TYPE(UnexpectedFileFormatError, WrongPathError);
    REGISTER_EXCEPTION_TYPE(FileNotFoundError,         WrongPathError);
    REGISTER_EXCEPTION_TYPE(ConfigNotFoundError,       FileNotFoundError);

} // namespace exceptions

#endif //SERVER_ERRORS_H
