#include <latercli.hpp>

auto main(int argc, char **argv) -> int {
    Later::CLIApplication app(argc, argv);
    return app.exec();
}
