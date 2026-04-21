#include "lines/temporal/clocks.hpp"
#include "lines/temporal/duration.hpp"
#include "lines/temporal/timestamp.hpp"
#include "lines/temporal/ymd.hpp"
#include <algorithm>
#include <client-utils/parsers.hpp>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <regex>
#include <stdexcept>
#include <string_view>

void range_error(std::string_view prefix, std::string_view range_str) {
    throw std::out_of_range(std::format("ERROR: {} can be only in range {}", prefix, range_str));
}

namespace {
auto to_lower_str(std::string str) -> std::string {
    std::ranges::transform(str, str.begin(),
                           [](unsigned char c) -> int { return std::tolower(c); });
    return str;
}

struct TimeExprSplitResult {
    std::string base;
    std::string operators;
};

struct TimeOperator {
    int value;
    char unit;
};

void eval_date_operators(Lines::Temporal::Date &date, const std::vector<TimeOperator> &ops) {
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
                std::format("ERROR: Unknown unit: {}. Only units are usable with relative dates "
                            "are y(years), m(months), w(weeks) and d(days)",
                            op.unit));
        }
    }
}

void eval_time_operators(Lines::Temporal::Timestamp &ts, const std::vector<TimeOperator> &ops) {
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

// Parse operators after relative time point (e.g +1d+1m-1y... or +1h-1m+1s...)
auto parse_time_operators(std::string_view str) -> std::vector<TimeOperator> {
    std::vector<TimeOperator> res;

    std::size_t i{};
    while (i < str.size()) {
        int sign{};
        int num{};
        char unit{};

        if (str[i] != '+' && str[i] != '-') {
            throw std::invalid_argument(
                std::format("ERROR: Unknown operator: {}. Only operations "
                            "permitted with relative time points are + and -",
                            str[i]));
        }
        sign = (str[i] == '+') ? 1 : -1;
        ++i;

        bool has_digits{};

        while (i < str.size() && (std::isdigit(static_cast<unsigned char>(str[i])) != 0)) {
            has_digits = true;
            num = (num * 10) + (str[i] - '0');
            if (num > UINT16_MAX) {
                range_error("Number in relative operator", std::format("[0,{}]", UINT16_MAX));
            }
            ++i;
        }

        if (!has_digits) {
            throw std::invalid_argument("ERROR: No value given to time operator");
        }

        if (num > INT_MAX) {
        }

        if (i >= str.size()) {
            throw std::invalid_argument("ERROR: Missing unit in time expression");
        }
        unit = str[i];

        res.emplace_back(num * sign, std::tolower(static_cast<unsigned char>(unit)));
        ++i;
    }
    return res;
}

/* Splits time expressions like "1970.01.01+1d" or "12:34:56-56s" to base (before operators) and
operators */
auto split_time_expression(const std::string &str) -> TimeExprSplitResult {
    TimeExprSplitResult res;

    std::size_t first_operator = str.find_first_of("+-");
    if (first_operator != std::string::npos) {
        res.base = str.substr(0, first_operator);
        res.operators = str.substr(first_operator);
        return res;
    }
    res.base = str;
    return res;
}

auto parse_date_base(const std::string &str) -> Lines::Temporal::Date {
    auto lower_str = to_lower_str(str);

    if (lower_str == "today" || lower_str == "t") {
        return Lines::Temporal::LocalClock::today();
    }

    std::stringstream ss(str);
    int year{};
    uint32_t month{};
    uint32_t day{};
    char divider{};

    ss >> year >> divider >> month >> divider >> day;

    Lines::Temporal::Year t_year{year};
    Lines::Temporal::Month t_month{month};
    Lines::Temporal::Day t_day{day};

    assert(t_year.ok());
    if (!t_month.ok()) {
        range_error("Month", "[1;12]");
    }
    if (!t_day.ok()) {
        range_error("Day", "[1;31]");
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
        range_error("Hour", "[0;23]");
    }

    if (minute < 0 || minute > 59) {
        range_error("Minute", "[0;59]");
    }

    if (second < 0 || second > 59) {
        range_error("Second", "[0;59]");
    }

    return {Lines::Temporal::Hours{hour}, Lines::Temporal::Minutes{minute},
            Lines::Temporal::Seconds{second}};
}
} // namespace

auto parse_date(const std::string &str) -> Lines::Temporal::Date {
    static const std::regex date_regex(R"(^(\d{4}\.\d{2}\.\d{2}|today|t)([+-]\d+[ymwd])*$)",
                                       std::regex::icase);
    if (!std::regex_match(str, date_regex)) {
        throw std::invalid_argument(
            R"(ERROR: Unknown date format.
Supported date formats:

Absolute:
  YYYY.MM.DD
  YYYY.MM.DD[operators...]

Relative:
  TODAY | T
  TODAY[operators...]

Operators:
  +N[ymwd]  add time
  -N[ymwd]  subtract time)");
    }

    auto expr = split_time_expression(str);
    Lines::Temporal::Date res{Lines::Temporal::Days{0}};

    res = parse_date_base(expr.base);

    eval_date_operators(res, parse_time_operators(expr.operators));

    return res;
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

    auto expr = split_time_expression(str);
    Lines::Temporal::Timestamp res = parse_time_base(expr.base);

    eval_time_operators(res, parse_time_operators(expr.operators));

    return res;
}

auto parse_timepoint(const std::string &str) -> Lines::Temporal::TimePoint {
    std::size_t middle_divider = str.find('_');

    std::string date = str.substr(0, middle_divider);
    std::string time = str.substr(middle_divider + 1);

    return Lines::Temporal::DateTime{parse_date(date), parse_time(time)}.time_point();
}
