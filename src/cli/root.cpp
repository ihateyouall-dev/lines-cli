#include <cli/root.hpp>
#include <lines/version.h>

Root::Root() {
    add_flag_callback("-v,--version", [] { std::cout << "Lines CLI " << LINES_VERSION << '\n'; });
    _tasks.init(*this);
    this->callback([this] {
        if (_tasks.dirty()) {
            _tasks.save();
        }
    });
}
