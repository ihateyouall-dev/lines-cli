#include <cli/root.hpp>

Root::Root() {
    add_flag_callback("-v,--version", [] { std::cout << "Lines CLI " << LINES_VERSION << '\n'; });
}
