#include <cli/root.hpp>
#include <storages/tasks/json.hpp>

auto main(int argc, char **argv) -> int {
    Root root;
    CLI11_PARSE(root, argc, argv);
    return 0;
}
