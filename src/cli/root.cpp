#include "cli/root.hpp"

#include "cli/docs/docs.hpp"
#include "lines/version.h"

Root::Root() {
    add_flag_callback("-v,--version",
                      []() -> void { std::cout << "Lines CLI " << LINES_VERSION << '\n'; });
    _tasks.init(*this);
    _docs.init(*this);
    this->callback([this]() -> void {
        if (_tasks.dirty()) {
            _tasks.save();
        }
    });
}
