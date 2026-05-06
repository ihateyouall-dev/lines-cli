#include "client-utils/utils.hpp"

#include "lines/temporal/clocks.hpp"

#include <format>
#include <iostream>

auto confirm() -> bool {
    std::string answer{};
    std::cout << "\n\nAre you sure? [yN]: ";
    std::getline(std::cin, answer);
    if (answer.empty()) {
        return false;
    }
    return std::tolower(static_cast<unsigned char>(answer[0])) == 'y';
}

auto date_str(const Lines::Temporal::Date &date) -> std::string {
    return std::format("{:04}/{:02}/{:02}", int(date.year()), unsigned(date.month()),
                       unsigned(date.day()));
}

auto timepoint_str(const Lines::Temporal::TimePoint &tp) -> std::string {
    Lines::Temporal::DateTime dt(tp);
    return std::format("{} {}", date_str(dt.date()), dt.time().hh_mm_ss());
}

auto timepoint_str_s(const Lines::Temporal::TimePoint &tp) -> std::string {
    Lines::Temporal::DateTime dt(tp);
    return std::format("{}_{}", date_str(dt.date()), dt.time().hh_mm_ss());
}

auto tags_str(const Lines::Task &task) -> std::string {
    std::string result;
    for (const auto &tag : task.tags()) {
        result += std::format(" #{}", tag);
    }
    return result;
}

auto completion_sign(const Lines::Task &task) -> std::string {
    std::string sign{};
    assert(!task.completed() ||
           !task.repeat_rule() && "Tasks with repeat rule cannot have completion state");
    if (task.repeat_rule()) {
        sign = "↻";
    } else if (task.completed()) {
        sign = "✓";
    } else {
        sign = " ";
    }
    return std::format("[{}]", sign);
}

auto task_str_unfolded(const Lines::Task &task) -> std::string {
    std::string result = std::format("Title: {}\n", task.title());
    if (task.description()) {
        if (!task.description().value().empty()) {
            result += std::format("Description: {}\n", task.description().value());
        }
    }
    if (!task.tags().empty()) {
        result += std::format("Tags:{}\n", tags_str(task));
    }
    if (task.deadline()) {
        result += std::format("Deadline: {}\n", timepoint_str(*task.deadline()));
    }
    if (task.repeat_rule() && task.next_deadline()) {
        result += std::format("Next deadline: {}\n", timepoint_str(*task.next_deadline()));
    }
    result += '\n' + completion_sign(task);
    return result;
}

auto task_str(const Lines::Task &task) -> std::string {
    std::string res = std::format("{} ", completion_sign(task));
    res += task.title();
    if (task.deadline()) {
        res += std::format(" {}", timepoint_str(*task.deadline()));
    }
    if (!task.tags().empty()) {
        res += tags_str(task);
    }
    return res;
}

auto today() -> Lines::Temporal::Date { return Lines::Temporal::LocalClock::today(); }
auto today_str() -> std::string { return date_str(today()); }

auto tomorrow() -> Lines::Temporal::Date { return today() + Lines::Temporal::Days{1}; }
auto tomorrow_str() -> std::string { return date_str(tomorrow()); }
