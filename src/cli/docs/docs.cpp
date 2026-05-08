#include "cli/docs/docs.hpp"

#include "CLI/CLI11.hpp"
#include "manuals/manuals.hpp"

void Lines::CLI::Docs::init(::CLI::App &app) {
    auto *docs = app.add_subcommand("docs", "Read official documentation");
    docs->add_option("manual", manual,
                     "Choose manual to read (Possible values: repeat, timepoints)")
        ->required()
        ->check(::CLI::IsMember({"repeat", "timepoints"}));

    docs->callback([this]() -> void {
        if (manual == "repeat") {
            std::cout << Lines::Manuals::repeat_manual << '\n';
            return;
        }
        if (manual == "timepoints") {
            std::cout << Lines::Manuals::timepoints_manual << '\n';
            return;
        }
    });
}
