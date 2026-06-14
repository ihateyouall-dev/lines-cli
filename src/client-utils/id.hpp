#pragma once

#include <cstddef>
#include <type_traits>

namespace Lines::ClientUtils {
template <typename Rep, Rep Primary,
          typename = std::enable_if_t<std::is_integral_v<Rep>>>
// Represents constant ID with primary point of count
class ID {
    Rep _rep;

  public:
    using value_type = Rep;
    static constexpr Rep primary = Primary;

    constexpr ID() : _rep{primary} {}
    constexpr explicit ID(Rep rep) : _rep(rep) {}

    constexpr explicit operator Rep() const noexcept { return _rep; }

    constexpr auto operator+(const ID &rhs) const noexcept -> ID {
        return ID(_rep + rhs._rep);
    }
    constexpr auto operator-(const ID &rhs) const noexcept -> ID {
        return ID(_rep - rhs._rep);
    }
    constexpr auto operator*(const ID &rhs) const noexcept -> ID {
        return ID(_rep * rhs._rep);
    }
    constexpr auto operator/(const ID &rhs) const noexcept -> ID {
        return ID(_rep / rhs._rep);
    }
    constexpr auto operator%(const ID &rhs) const noexcept -> ID {
        return ID(_rep % rhs._rep);
    }

    constexpr auto operator+=(const ID &rhs) noexcept -> ID & {
        _rep += rhs._rep;
        return *this;
    }
    constexpr auto operator-=(const ID &rhs) noexcept -> ID & {
        _rep -= rhs._rep;
        return *this;
    }
    constexpr auto operator*=(const ID &rhs) noexcept -> ID & {
        _rep *= rhs._rep;
        return *this;
    }
    constexpr auto operator/=(const ID &rhs) noexcept -> ID & {
        _rep /= rhs._rep;
        return *this;
    }
    constexpr auto operator%=(const ID &rhs) noexcept -> ID & {
        _rep %= rhs._rep;
        return *this;
    }

    friend constexpr auto operator<=>(const ID &lhs, const ID &rhs) {
        return lhs._rep <=> rhs._rep;
    }
};

template <typename> struct is_id : std::false_type {};

template <typename Rep, Rep Primary>
struct is_id<ID<Rep, Primary>> : std::true_type {};

template <typename Tp> static constexpr bool is_id_v = is_id<Tp>::value;

template <typename To, typename From,
          typename = std::enable_if_t<is_id_v<From> && is_id_v<To>>> // NOLINT
constexpr auto id_cast(From from) -> To {
    return To{typename From::value_type(from) + (To::primary - From::primary)};
}
} // namespace Lines::ClientUtils

static_assert(
    Lines::ClientUtils::is_id_v<Lines::ClientUtils::ID<std::size_t, 0>>);
