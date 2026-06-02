#include "cli/root.hpp"

#include "cli/docs/docs.hpp"
#include "version.h"

Lines::CLI::Root::Root() {
    add_flag_callback("-v,--version",
                      []() -> void { std::cout << "Lines CLI " << LINES_CLI_VERSION_STR << '\n'; });
    _config.init(*this);
    _tasks.set_config(_config.config());
    _tasks.init(*this);
    _docs.init(*this);
    this->callback([this]() -> void {
        if (_tasks.dirty()) {
            _tasks.save();
        }
        _config.save();
    });
}
