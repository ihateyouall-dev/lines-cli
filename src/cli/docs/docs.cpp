#include "cli/docs/docs.hpp"

#include "CLI/CLI11.hpp"

void Docs::init(CLI::App &app) {
    auto *docs = app.add_subcommand("docs", "Read official documentation");
    docs->add_option("manual", manual,
                     "Choose manual to read (Possible values: repeat, timepoints)")
        ->required();

    docs->callback([this]() -> void {
        static const std::string repeat_manual = R"(Task repeat rules.

The difference between regular task and task with repeat rule is that
repeatable tasks do not have "completed" or "uncompleted" state.
Every time you complete a repeatable task, it only moves task's deadline
to the next occurrence, which is calculated using repeat rule.

There are two types of repeat rules:
	1. Every N units.   (Like "every 5 days", "every week", "every 7 hours", etc.)
	2. Every <weekday>. (Like "every monday", "every saturday and sunday",
                                                  "every tuesday to friday", etc.)
Syntax:
	1. N(s|m|h|d|w|mo|y).
           Examples: "5d", "1w", "7h", "3mo"

	2. wd1,wd2  ->  multiple weekdays (WD1 AND WD2)
           Example: "mon,wed"
           wd1.wd2  ->  weekday range (FROM WD1 TO WD2)
           Example: "mon.wed"

           Weekday names:
           mon tue wed thu fri sat sun

           Both can be combined in one rule, like "mon,wed.sat")";

        if (manual == "repeat") {
            std::cout << repeat_manual << '\n';
        }
    });
}
