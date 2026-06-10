#include "client-utils/utils.hpp"

#include "client-utils/colors.hpp"
#include "lines/temporal/clocks.hpp"
#include "lines/temporal/timepoint.hpp"

#include <cassert>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

namespace Lines::ClientUtils {
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
    return std::format("{:04}/{:02}/{:02}", int(date.year()),
                       unsigned(date.month()), unsigned(date.day()));
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

auto completion_sign(const Lines::Task &task, bool use_unicode, bool colorize)
    -> std::string {
    std::string sign{};
    assert(!task.completed() ||
           !task.repeat_rule() &&
               "Tasks with repeat rule cannot have completion state");
    auto process_sign = [&](std::string &sign, std::string_view full,
                            std::string_view fallback, // NOLINT
                            const std::string &color) -> void {
        sign = full;
        if (!use_unicode) {
            sign = fallback;
        }
        if (colorize) {
            sign = Colors::colorize(sign, color);
        }
    };
    if (task.repeat_rule()) {
        process_sign(sign, "↻", "R", Colors::blue);
    } else if (task.completed()) {
        process_sign(sign, "✓", "X", Colors::green);
    } else {
        sign = " ";
    }
    return std::format("[{}]", sign);
}

static auto get_due_str_func(bool colorize) // NOLINT
    -> std::function<std::string(Lines::Temporal::TimePoint)> {
    std::function<std::string(Lines::Temporal::TimePoint)> res = due_str;
    if (!colorize) {
        res = timepoint_str;
    }
    return res;
}

auto full_task_str(const Lines::Task &task, bool use_unicode, bool colorize)
    -> std::string {
    std::string result = std::format("Title: {}\n", task.title());
    if (task.description()) {
        if (!task.description().value().empty()) {
            result +=
                std::format("Description: {}\n", task.description().value());
        }
    }
    if (!task.tags().empty()) {
        result += std::format("Tags:{}\n", tags_str(task));
    }
    auto due_str_func = get_due_str_func(colorize);
    if (task.due()) {
        result += std::format("Due: {}\n", due_str_func(*task.due()));
    }
    if (task.repeat_rule()) {
        if (task.next_due()) {
            result +=
                std::format("Next due: {}\n", due_str_func(*task.next_due()));
        }
        auto rr = *task.repeat_rule();
        if (rr.end) {
            result += std::format("Repeat ends: {}\n", due_str_func(*rr.end));
        }
    }
    result += '\n' + completion_sign(task, use_unicode, colorize);
    return result;
}

auto task_str(const Lines::Task &task, bool use_unicode, bool colorize)
    -> std::string {
    std::string res =
        std::format("{} ", completion_sign(task, use_unicode, colorize));
    res += task.title();
    if (task.due()) {
        auto due_str_func = get_due_str_func(colorize);
        res += std::format(" {}", due_str_func(*task.due()));
    }
    if (!task.tags().empty()) {
        res += tags_str(task);
    }
    return res;
}

auto today() -> Lines::Temporal::Date {
    return Lines::Temporal::LocalClock::today();
}
auto today_str() -> std::string { return date_str(today()); }

auto tomorrow() -> Lines::Temporal::Date {
    return today() + Lines::Temporal::Days{1};
}
auto tomorrow_str() -> std::string { return date_str(tomorrow()); }

auto due_color(const Lines::Temporal::TimePoint &due) -> std::string {
    using namespace Lines::Temporal::Literals;

    auto delta = due - Lines::Temporal::LocalClock::now();

    if (delta < 0_s) {
        return Colors::red;
    }
    if (delta <= 24_h) {
        return Colors::yellow;
    }
    return Colors::green;
}

auto due_str(const Lines::Temporal::TimePoint &due) -> std::string {
    return Colors::colorize(timepoint_str(due), due_color(due));
}

auto error_str(const std::string &prefix, std::string_view msg) -> std::string {
    return std::format("{} {}", Colors::colorize(prefix, Colors::red), msg);
}
} // namespace Lines::ClientUtils
