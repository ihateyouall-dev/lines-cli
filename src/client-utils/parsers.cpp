#include "client-utils/parsers.hpp"

#include "lines/detail/macro.h"
#include "lines/tasks/task_repeat.hpp"
#include "lines/temporal/clocks.hpp"
#include "lines/temporal/date.hpp"
#include "lines/temporal/datetime.hpp"
#include "lines/temporal/duration.hpp"
#include "lines/temporal/timepoint.hpp"
#include "lines/temporal/ymd.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

void Lines::ClientUtils::Parsers::throw_range_error(std::string_view prefix,
                                                    std::string_view range_str) {
    throw std::out_of_range(std::format("ERROR: {} must be in range {}", prefix, range_str));
}

namespace {
auto to_lower_str(std::string str) -> std::string {
    std::ranges::transform(str, str.begin(),
                           [](unsigned char c) -> int { return std::tolower(c); });
    return str;
}

struct TemporalExprSplitResult {
    std::string base;
    std::string operators;
};

struct TemporalOperator {
    int value;
    char unit;
};

void eval_date_operators(Lines::Temporal::Date &date, const std::vector<TemporalOperator> &ops) {
    for (const auto &op : ops) {
        switch (op.unit) {
        case 'y':
            date += Lines::Temporal::Years{op.value};
            break;
        case 'm':
            date += Lines::Temporal::Months{op.value};
            break;
        case 'w':
            date += Lines::Temporal::Weeks{op.value};
            break;
        case 'd':
            date += Lines::Temporal::Days{op.value};
            break;
        default:
            throw std::invalid_argument(
                std::format("ERROR: Unknown unit: {}. Only units are usable with date operators "
                            "are y(years), m(months), w(weeks) and d(days)",
                            op.unit));
        }
    }
}

void eval_time_operators(Lines::Temporal::Timestamp &ts, const std::vector<TemporalOperator> &ops) {
    for (const auto &op : ops) {
        switch (op.unit) {
        case 'h':
            ts += Lines::Temporal::Hours{op.value};
            break;
        case 'm':
            ts += Lines::Temporal::Minutes{op.value};
            break;
        case 's':
            ts += Lines::Temporal::Seconds{op.value};
            break;
        default:
            throw std::invalid_argument(
                std::format("ERROR: Unknown unit: {}. Only units are usable with time operators "
                            "are h(hours), m(minutes) and s(seconds)",
                            op.unit));
        }
    }
}

// Parse operators after temporal expression base (e.g +1d+1m-1y... or +1h-1m+1s...)
auto parse_temporal_operators(std::string_view str) -> std::vector<TemporalOperator> {
    std::vector<TemporalOperator> res;

    std::size_t i{};
    while (i < str.size()) {
        int sign{};
        uint64_t num{};
        char unit{};

        if (str[i] != '+' && str[i] != '-') {
            throw std::invalid_argument(
                std::format("ERROR: Unknown operator: {}. Only operations "
                            "permitted with temporal expressions are + and -",
                            str[i]));
        }
        sign = (str[i] == '+') ? 1 : -1;
        ++i;

        bool has_digits{};

        while (i < str.size() && (std::isdigit(static_cast<unsigned char>(str[i])) != 0)) {
            has_digits = true;
            num = (num * 10) + static_cast<uint64_t>((str[i] - '0'));
            if (num > UINT16_MAX) {
                Lines::ClientUtils::Parsers::throw_range_error("Value in operator",
                                                               std::format("[0,{}]", UINT16_MAX));
            }
            ++i;
        }

        if (!has_digits) {
            throw std::invalid_argument("ERROR: No value given to temporal operator");
        }

        if (i >= str.size()) {
            throw std::invalid_argument("ERROR: Missing unit in temporal expression");
        }
        unit = str[i];

        res.emplace_back(static_cast<int>(num) * sign,
                         std::tolower(static_cast<unsigned char>(unit)));
        ++i;
    }
    return res;
}

/* Splits temporal expressions like "1970/01/01+1d" or "12:34:56-56s" to base (before operators) and
operators */
auto split_temporal_expression(const std::string &str) -> TemporalExprSplitResult {
    TemporalExprSplitResult res;

    std::size_t first_operator = str.find_first_of("+-");
    if (first_operator != std::string::npos) {
        res.base = str.substr(0, first_operator);
        res.operators = str.substr(first_operator);
        return res;
    }
    res.base = str;
    return res;
}

auto days_in_month(Lines::Temporal::Month month) -> std::size_t {
    static constexpr std::array<std::size_t, 12> days = {31, 28, 31, 30, 31, 30,
                                                         31, 31, 30, 31, 30, 31};
    return days[unsigned(month) - 1]; // NOLINT
}

auto parse_date_base(const std::string &str) -> Lines::Temporal::Date {
    auto base = to_lower_str(str);

    using namespace Lines::Temporal;
    using DateBaseFunc = Date (*)();
    static const std::unordered_map<std::string_view, DateBaseFunc> base_funcs{
        {"today", []() -> Date { return LocalClock::today(); }},
        {"tomorrow", []() -> Date { return LocalClock::today() + Days{1}; }},
        {"yesterday", []() -> Date { return LocalClock::today() - Days{1}; }}};

    // If base has function in base_funcs, it returns function value, otherwise it parses base as
    // date in format YYYY/MM/DD
    if (auto it = base_funcs.find(base); it != base_funcs.end()) {
        return it->second();
    }

    std::stringstream ss(str);
    int year{};
    uint32_t month{};
    uint32_t day{};
    char divider{};

    ss >> year >> divider >> month >> divider >> day;

    Year t_year{year};
    Month t_month{month};
    Day t_day{day};

    assert(t_year.ok());
    if (!t_month.ok()) {
        Lines::ClientUtils::Parsers::throw_range_error("Month", "[1;12]");
    }
    std::size_t max_days = days_in_month(t_month);
    if (t_year.is_leap() && t_month == Lines::Temporal::Month{2}) {
        ++max_days;
    }
    if (unsigned(t_day) > max_days) {
        static const std::array<std::string, 12> month_str{
            "January", "February", "March",     "April",   "May",      "June",
            "July",    "August",   "September", "October", "November", "December"};
        Lines::ClientUtils::Parsers::throw_range_error(
            std::format("Day in {} of {}", month_str[unsigned(t_month) - 1], int(t_year)), // NOLINT
            std::format("[1;{}]", max_days));
    }

    return {t_year, t_month, t_day};
}

auto parse_time_base(const std::string &str) -> Lines::Temporal::Timestamp {
    auto lower_str = to_lower_str(str);

    if (lower_str == "now") {
        return Lines::Temporal::LocalClock::since_midnight();
    }

    std::stringstream ss(str);
    int hour{};
    int minute{};
    int second{};
    char divider{};

    ss >> hour >> divider >> minute >> divider >> second;

    if (hour < 0 || hour > 23) {
        Lines::ClientUtils::Parsers::throw_range_error("Hour", "[0;23]");
    }

    if (minute < 0 || minute > 59) {
        Lines::ClientUtils::Parsers::throw_range_error("Minute", "[0;59]");
    }

    if (second < 0 || second > 59) {
        Lines::ClientUtils::Parsers::throw_range_error("Second", "[0;59]");
    }

    return {Lines::Temporal::Hours{hour}, Lines::Temporal::Minutes{minute},
            Lines::Temporal::Seconds{second}};
}

auto parse_repeat_interval(std::size_t val, std::string unit) -> Lines::Temporal::Seconds {
    using namespace Lines::Temporal;

    const auto v = static_cast<int64_t>(val);

    unit = to_lower_str(unit);

    if (unit == "s") {
        return Seconds{v};
    }
    if (unit == "m") {
        return duration_cast<Seconds>(Minutes{v});
    }
    if (unit == "h") {
        return duration_cast<Seconds>(Hours{v});
    }
    if (unit == "d") {
        return duration_cast<Seconds>(Days{v});
    }
    if (unit == "w") {
        return duration_cast<Seconds>(Weeks{v});
    }
    if (unit == "mo") {
        return duration_cast<Seconds>(Months{v});
    }
    if (unit == "y") {
        return duration_cast<Seconds>(Years{v});
    }

    throw std::invalid_argument(R"(ERROR: Invalid repeat unit
Expected units:
s - seconds
m - minutes
h - hours
d - days
w - weeks
mo - months
y - years

See "lines-cli docs repeat" for more info)");
}

auto parse_weekday(std::string_view str) -> Lines::Temporal::Weekday {
    if (str == "mon") {
        return Lines::Temporal::Weekday::Monday;
    }
    if (str == "tue") {
        return Lines::Temporal::Weekday::Tuesday;
    }
    if (str == "wed") {
        return Lines::Temporal::Weekday::Wednesday;
    }
    if (str == "thu") {
        return Lines::Temporal::Weekday::Thursday;
    }
    if (str == "fri") {
        return Lines::Temporal::Weekday::Friday;
    }
    if (str == "sat") {
        return Lines::Temporal::Weekday::Saturday;
    }
    if (str == "sun") {
        return Lines::Temporal::Weekday::Sunday;
    }
    LINES_UNREACHABLE();
}

auto parse_repeat_weekday(std::string_view str) -> std::vector<Lines::Temporal::Weekday> {
    auto point = str.find('.');
    if (point == std::string_view::npos) {
        return {parse_weekday(str)};
    }
    if (str.size() != 7 || str[3] != '.') {
        throw std::invalid_argument(R"(ERROR: Invalid weekday format.
Examples:
mon
wed.sat

See "lines-cli docs repeat" for more info)");
    }
    std::vector<Lines::Temporal::Weekday> res;
    auto begin = parse_weekday(str.substr(0, point));
    auto end = parse_weekday(str.substr(point + 1));

    auto next_weekday = [](auto wd) -> Lines::Temporal::Weekday {
        if (wd == Lines::Temporal::Weekday::Sunday) {
            return Lines::Temporal::Weekday::Monday;
        }

        return static_cast<Lines::Temporal::Weekday>(static_cast<uint8_t>(wd) + 1);
    };

    for (auto wd = begin;; wd = next_weekday(wd)) {
        res.push_back(wd);
        if (wd == end) {
            break;
        }
    }
    return res;
}

// Parses repeat rule with weekdays, like "mon,wed.sat"
// wd1,wd2 means "WD1 and WD2"
// wd1.wd2 means "From WD1 to WD2"
auto parse_repeat_weekdays(std::string str) -> std::vector<Lines::Temporal::Weekday> {
    str = to_lower_str(str);
    std::vector<Lines::Temporal::Weekday> res;

    std::stringstream ss(str);
    std::string wds;
    while (std::getline(ss, wds, ',')) {
        for (const auto &el : parse_repeat_weekday(wds)) {
            res.push_back(el);
        }
    }
    return res;
}
} // namespace

namespace Lines::ClientUtils::Parsers {
// Date parser without regex validation
auto parse_date_nv(const std::string &str) -> Lines::Temporal::Date {
    auto expr = split_temporal_expression(str);
    Lines::Temporal::Date res{Lines::Temporal::Days{0}};

    res = parse_date_base(expr.base);

    eval_date_operators(res, parse_temporal_operators(expr.operators));

    return res;
}
// Same for time
auto parse_time_nv(const std::string &str) -> Lines::Temporal::Timestamp {
    auto expr = split_temporal_expression(str);
    Lines::Temporal::Timestamp res = parse_time_base(expr.base);

    eval_time_operators(res, parse_temporal_operators(expr.operators));

    return res;
}

auto parse_date(const std::string &str) -> Lines::Temporal::Date {
    static const std::regex date_regex(
        R"(^(\d{4}/\d{2}/\d{2}|today|tomorrow|yesterday)([+-]\d+[ymwd])*$)", std::regex::icase);
    if (!std::regex_match(str, date_regex)) {
        throw std::invalid_argument(
            R"(ERROR: Unknown date format.
Supported date formats:

Absolute:
  YYYY/MM/DD
  YYYY/MM/DD[operators...]

Relative:
  TODAY
  TODAY[operators...]

Operators:
  +N[ymwd]  add time
  -N[ymwd]  subtract time)");
    }

    return parse_date_nv(str);
}

auto parse_time(const std::string &str) -> Lines::Temporal::Timestamp {
    static const std::regex time_regex(R"(^(\d{2}:\d{2}(:\d{2})?|now)([+-]\d+[hms])*$)",
                                       std::regex::icase);

    if (!std::regex_match(str, time_regex)) {
        throw std::invalid_argument(
            R"(ERROR: Unknown time format.
Supported time formats:

Absolute:
  HH:MM:SS
  HH:MM:SS[operators...]
  HH:MM
  HH:MM[operators...]

Relative:
  NOW
  NOW[operators...]

Operators:
  +N[hms]  add time
  -N[hms]  subtract time)");
    }
    return parse_time_nv(str);
}

auto parse_timepoint_nv(const std::string &str) -> Lines::Temporal::TimePoint {
    std::size_t middle_divider = str.find('_');

    std::string date = str.substr(0, middle_divider);

    if (middle_divider == std::string::npos) {
        return Lines::Temporal::DateTime{parse_date_nv(date),
                                         Lines::Temporal::Timestamp{Lines::Temporal::Seconds{-1}}}
            .time_point();
    }

    std::string time = str.substr(middle_divider + 1);

    return Lines::Temporal::DateTime{parse_date_nv(date), parse_time_nv(time)}.time_point();
}

auto parse_timepoint(const std::string &str) -> Lines::Temporal::TimePoint {
    std::size_t middle_divider = str.find('_');

    std::string date = str.substr(0, middle_divider);

    if (middle_divider == std::string::npos) {
        return Lines::Temporal::DateTime{parse_date(date),
                                         Lines::Temporal::Timestamp{Lines::Temporal::Seconds{-1}}}
            .time_point();
    }

    std::string time = str.substr(middle_divider + 1);

    return Lines::Temporal::DateTime{parse_date(date), parse_time(time)}.time_point();
}

auto parse_repeat_rule(const std::string &str) -> Lines::TaskRepeatRule {
    static const std::regex every_unit_regex(R"(^(\d+)(y|mo|w|d|h|m|s)$)", std::regex::icase);
    static const std::regex every_weekday_regex(
        R"(^((mon|tue|wed|thu|fri|sat|sun)[,.])*(mon|tue|wed|thu|fri|sat|sun)$)",
        std::regex::icase);

    std::smatch groups;
    if (std::regex_match(str, groups, every_unit_regex)) {
        std::size_t val = static_cast<std::size_t>(std::stoi(groups[1]));
        std::string unit = groups[2];
        Lines::TaskRepeatRule res;
        res.repeat_type = Lines::TaskRepeat::EveryUnit{.interval = parse_repeat_interval(val, unit),
                                                       .unit_str = unit};
        return res;
    }
    if (!std::regex_match(str, every_weekday_regex)) {
        throw std::invalid_argument(R"(ERROR: Invalid repeat rule format. Formats:
<N>unit     (e.g. "3d" repeats every 3 days)
wd1,wd2.wd3 (e.g. "mon,wed.sun" repeats on monday and from wednesday to sunday

See "lines-cli docs repeat" for more info)");
    }

    Lines::TaskRepeatRule res;
    res.repeat_type = Lines::TaskRepeat::EveryWeekday{.weekdays = parse_repeat_weekdays(str)};
    return res;
}
} // namespace Lines::ClientUtils::Parsers
