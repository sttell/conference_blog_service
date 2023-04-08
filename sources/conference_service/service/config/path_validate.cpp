#include "path_validate.h"

#include <array>
#include <filesystem>
#include <functional>

#include "errors.h"

#define ERROR_CHECK_PREFIX "Error by provided path: "

namespace {

    constexpr const char* kExpectedFileExtension = ".json";

} // namespace [ constants ]

struct FailReason;
using PathCheckFunctor = std::function<FailReason(const std::filesystem::path&)>;

struct FailReason {

    enum class Code {
        OK,
        NOT_FOUND,
        UNEXPECTED_FORMAT,
        UNEXPECTED_PATH
    };

    std::string description;
    Code code{ Code::OK };
};

FailReason CheckExists (const std::filesystem::path& path) noexcept {
    if ( !std::filesystem::exists(path) ) {
        FailReason reason;
        reason.description = ERROR_CHECK_PREFIX + path.string() + " file not exists";
        reason.code = FailReason::Code::NOT_FOUND;
        return reason;
    }
    return FailReason{};
}

FailReason CheckHasJsonExtension (const std::filesystem::path& path) noexcept {
    if ( path.extension() != kExpectedFileExtension ) {
        FailReason reason;
        reason.description = ERROR_CHECK_PREFIX + path.string() + ". File must be only .json format.";
        reason.code = FailReason::Code::UNEXPECTED_FORMAT;
    }
    return FailReason{};
}

FailReason CheckIsFile (const std::filesystem::path& path) noexcept {

    if ( !std::filesystem::is_regular_file(path) ) {
        FailReason reason;
        reason.description = ERROR_CHECK_PREFIX + path.string() + ". Expected path to file.";
        reason.code = FailReason::Code::UNEXPECTED_PATH;
        return reason;
    }
    return FailReason{};
}

inline void ThrowFailReason(const FailReason& reason) {
    switch (reason.code) {
        case FailReason::Code::OK: break;
        case FailReason::Code::NOT_FOUND:
            throw exceptions::ConfigNotFoundError(reason.description);
        case FailReason::Code::UNEXPECTED_FORMAT:
            throw exceptions::UnexpectedFileFormatError(reason.description);
        case FailReason::Code::UNEXPECTED_PATH:
            throw exceptions::WrongPathError(reason.description);
    }
}

namespace config::utils {

    void ValidateJsonPath(const std::string &path) {
        std::filesystem::path fs_path{path};
        static const std::array<PathCheckFunctor, 3> checks{
            CheckExists,
            CheckIsFile,
            CheckHasJsonExtension
        };

        for ( const auto& check : checks ) {
            auto reason = check(fs_path);
            ThrowFailReason(reason);
        }
    }

} // namespace config::utils