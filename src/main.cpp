#include "cli/root.hpp"

auto main(int argc, char **argv) -> int {
    Root root;
    CLI11_PARSE(root, argc, argv);
    return 0;
}
