#pragma once

#include <optional>
#include <string>

namespace git {

    template <typename Enum>
    constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept
    {
        return static_cast<std::underlying_type_t<Enum>>(e);
    }

    template <typename T>
    std::optional<T*> as_optional(T* ptr) noexcept
    {
        return ptr ? std::optional{ptr} : std::nullopt;
    }

    void throw_on_git2_error(int error);

    [[noreturn]] void throw_with_message(std::string const& message);

}
