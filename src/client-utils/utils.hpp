#pragma once

#include "client-utils/colors.hpp"
#include "lines/tasks/task.hpp"
#include "lines/temporal/timepoint.hpp"

#include <string>
#include <string_view>

namespace Lines::ClientUtils {
auto confirm() -> bool;

auto date_str(const Lines::Temporal::Date &date) -> std::string;

auto timepoint_str(const Lines::Temporal::TimePoint &tp) -> std::string;
auto timepoint_str_s(const Lines::Temporal::TimePoint &tp) -> std::string;

auto tags_str(const Lines::Task &task) -> std::string;

auto completion_sign(const Lines::Task &task, bool use_unicode = true,
                     bool colorize = true) -> std::string;

auto due_color(const Lines::Temporal::TimePoint &due) -> std::string;

auto due_str(const Lines::Temporal::TimePoint &due) -> std::string;

auto full_task_str(const Lines::Task &task, bool use_unicode, bool colorize)
    -> std::string;

auto task_str(const Lines::Task &task, bool use_unicode, bool colorize)
    -> std::string;

auto today() -> Lines::Temporal::Date;
auto today_str() -> std::string;

auto tomorrow() -> Lines::Temporal::Date;
auto tomorrow_str() -> std::string;

auto error_str(const std::string &prefix, std::string_view msg) -> std::string;
} // namespace Lines::ClientUtils
