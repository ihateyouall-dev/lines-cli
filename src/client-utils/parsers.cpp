#include "lines/temporal/clocks.hpp"
#include "lines/temporal/duration.hpp"
#include "lines/temporal/ymd.hpp"
#include <algorithm>
#include <client-utils/parsers.hpp>
#include <cstddef>
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

struct RelativeOperator {
    int64_t value;
    char unit;
};

void eval_relative_date(Lines::Temporal::Date &date, const std::vector<RelativeOperator> &ops) {
    for (const auto &op : ops) {
        switch (op.unit) {
        case 'y':
            date += Lines::Temporal::Years{op.value};
            break;
        case 'm':
            date += Lines::Temporal::Months{op.value};
            break;

        case 'd':
            date += Lines::Temporal::Days{op.value};
            break;
        default:
            throw std::invalid_argument(
                std::format("ERROR: Unknown unit: {}. Only units are usable with relative dates "
                            "are y(years), m(months) and d(days)",
                            op.unit));
        }
    }
}

// Parse operators after relative time point (e.g +1d+1m-1y... or +1h-1m+1s...)
auto parse_relative_operators(std::string_view str) -> std::vector<RelativeOperator> {
    std::vector<RelativeOperator> res;

    std::size_t i{};
    while (i < str.size()) {
        int8_t sign{};
        int64_t num{};
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
            ++i;
        }

        if (!has_digits) {
            throw std::invalid_argument("ERROR: No number given to relative date operator");
        }

        if (i >= str.size()) {
            throw std::invalid_argument("ERROR: Missing unit in relative date expression");
        }
        unit = str[i];

        res.emplace_back(num * sign, std::tolower(unit));
	++i;
    }
    return res;
}
} // namespace

auto parse_date(const std::string &str) -> Lines::Temporal::Date {
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

auto valid_relative_date(const std::string &str) -> bool {
    auto s = to_lower_str(str);
    return s.starts_with("t+") || s.starts_with("t-") || s.starts_with("today");
}

// Date parser for "TODAY+1y-1m+1d..." format
auto parse_relative_date(const std::string &str) -> Lines::Temporal::Date {
    Lines::Temporal::Date res = Lines::Temporal::LocalClock::today();

    auto tmp = to_lower_str(str);

    if (tmp == "today" || tmp == "t") {
        return res;
    }

    if (!valid_relative_date(str)) {
        throw std::invalid_argument(
            R"(ERROR: Relative date must start with "TODAY" or "T" (case insensitive))");
    }

    // Parsing operations after "TODAY" word
    std::size_t first_operator = str.find_first_of("+-");

    if (first_operator != std::string("today").length() &&
        first_operator != std::string("t").length()) {
        throw std::invalid_argument("ERROR: Invalid relative date expression");
    }

    std::string operations = str.substr(first_operator);

    std::vector<RelativeOperator> operators = parse_relative_operators(operations);

    eval_relative_date(res, operators);

    return res;
}

auto parse_time(const std::string &str) -> Lines::Temporal::Timestamp {
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

auto parse_timepoint(const std::string &str) -> Lines::Temporal::TimePoint {
    static const std::regex full_tp_regex(R"(\d{4}\.\d{2}\.\d{2}_\d{2}:\d{2}:\d{2})");
    static const std::regex date_only_tp_regex(R"(\d{4}\.\d{2}\.\d{2})");

    bool date_only_tp = std::regex_match(str, date_only_tp_regex);
    bool full_tp = std::regex_match(str, full_tp_regex);

    if (!full_tp && !date_only_tp) {
        throw std::invalid_argument(
            "ERROR: Invalid time point format, expected YYYY.MM.DD or YYYY.MM.DD_HH:MM:SS");
    }

    if (date_only_tp) {
        return Lines::Temporal::DateTime{parse_date(str),
                                         Lines::Temporal::Timestamp{Lines::Temporal::Seconds{0}}}
            .time_point();
    }

    std::size_t middle_divider = str.find('_');

    std::string date = str.substr(0, middle_divider);
    std::string time = str.substr(middle_divider + 1);

    return Lines::Temporal::DateTime{parse_date(date), parse_time(time)}.time_point();
}
