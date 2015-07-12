#include <iostream>
#include <fstream>
#include <sstream>
#include "snowman.hpp"

int main(int argc, char *argv[]) {
    // parse arguments
    std::string filename;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg[0] == '-') {
            char argid;
            if (arg.length() >= 2 && arg[1] == '-') {
                // no switch on strings :(
                if (arg == "--help") argid = 'h';
                else if (arg == "--interactive") argid = 'i';
                else {
                    std::cerr << "Unknown long argument `" << arg << "'" <<
                        std::endl;
                    return 1;
                }
            } else {
                if (arg.length() != 2) {
                    std::cerr << "Invalid syntax for argument `" << arg << "'"
                        << std::endl;
                    return 1;
                } else {
                    argid = arg[1];
                }
            }

            switch (argid) {
            // for future reference, this is how arguments with parameters work
            /*case 'i':
                if ((++i) == argc) {
                    std::cerr << "Argument `" << arg << "' requires a "
                        "parameter (ID)" << std::endl;
                    return 1;
                }
                try {
                    id = std::stoi(argv[i]);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Parameter `" << argv[i] << "' for argument `"
                        << arg << "' is not a number" << std::endl;
                    return 1;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Parameter `" << argv[i] << "' for argument `"
                        << arg << "' is out of range" << std::endl;
                    return 1;
                }
                break;*/
            case 'h':
                std::cout << "Usage: " << argv[0] << " [OPTION]... [FILENAME]\n" <<
                    "Options:\n"
                    "    -h, --help: (without filename) display this message\n"
                    "    -i, --interactive: (without filename) start a REPL\n"
                    "Snowman will read from STDIN if you do not specify a "
                        "file name or the -h or -i options.\n";
                return 0;
            case 'i':
                std::cout << "TODO: REPL" << std::endl;
                return 0;  // TODO
            default:
                std::cerr << "Unknown argument `" << arg << "'" << std::endl;
            }
        } else if (filename == "") {
            filename = arg;
        } else {
            std::cerr << "Multiple filenames specified (" << filename << ", "
                << arg << ")" << std::endl;
            return 1;
        }
    }

    std::string code;
    if (filename == "") {
        std::string line;
        while (std::getline(std::cin, line)) code += line;
    } else {
        std::ifstream infile(filename.c_str());
        if (infile.good()) {
            std::stringstream buf;
            buf << infile.rdbuf();
            code = buf.str();
        } else {
            std::cerr << "Could not read file " << filename << std::endl;
            return 1;
        }
    }

    Snowman sm = Snowman();
    sm.run(code);
}