#include <client-utils/parsers.hpp>
#include <regex>

void range_error(std::string_view prefix, std::string_view range_str) {
    throw std::invalid_argument(
        std::format("ERROR: {} can be only in range {}", prefix, range_str));
}

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
