#include <iostream>
#include <fstream>
#include <sstream>
#include "snowman.hpp"

int main(int argc, char *argv[]) {
    Snowman sm = Snowman();

    std::string VERSION_STRING = "v" + std::to_string(Snowman::MAJOR_VERSION) +
        "." + std::to_string(Snowman::MINOR_VERSION) + "." +
        std::to_string(Snowman::PATCH_VERSION);

    // parse arguments
    std::string filename;
    bool minify = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg[0] == '-') {
            arg = arg.substr(1);
            if (arg.length() >= 1 && arg[0] == '-') {
                arg = arg.substr(1);
                // no switch on strings :(
                if (arg == "help") arg = "h";
                else if (arg == "interactive") arg = "i";
                else if (arg == "debug") arg = "d";
                else if (arg == "evaluate") arg = "e";
                else if (arg == "minify") arg = "m";
                else {
                    std::cerr << "Unknown long argument `" << arg << "'" <<
                        std::endl;
                    return 1;
                }
            }

            for (char& argid : arg) {
                switch (argid) {
                case 'h':
                    std::cout << "Usage: " << argv[0] << " [OPTION]... "
                            "[FILENAME]\n" <<
                        "Options:\n"
                        "    -h, --help: (without filename) display this "
                            "message\n"
                        "    -i, --interactive: (without filename) start a "
                            "REPL\n"
                        "    -e, --evaluate: (without filename) takes one "
                            "parameter, runs as Snowman code\n"
                        "    -d, --debug: include debug output\n"
                        "    -m, --minify: don't evaluate code; output "
                            "minified version instead\n"
                        "Snowman will read from STDIN if you do not specify a "
                            "file name or the -h or -i options.\n"
                        "Snowman version: " << VERSION_STRING << "\n";
                    return 0;
                case 'i': {
                    std::cout << "Snowman REPL, " << VERSION_STRING <<
                        std::endl;
                    std::cout << ">> ";
                    std::string line;
                    while (std::getline(std::cin, line)) {
                        if (minify) {
                            for (std::string s : Snowman::tokenize(line)) {
                                std::cout << s;
                            }
                            std::cout << std::endl << ">> ";
                        } else {
                            sm.run(line);
                            std::cout << sm.debug();
                            std::cout << ">> ";
                        }
                    }
                    return 0;
                }
                case 'd':
                    sm.debugOutput = true;
                    break;
                case 'e':
                    if ((++i) == argc) {
                        std::cerr << "Argument `-e' requires a parameter" <<
                            std::endl;
                        return 1;
                    }
                    sm.run(std::string(argv[i]));
                    return 0;
                case 'm':
                    minify = true;
                    break;
                default:
                    std::cerr << "Unknown argument `-" << arg << "'" <<
                        std::endl;
                    return 1;
                }
            }
        } else if (filename == "") {
            filename = arg;
        } else {
            std::cerr << "Multiple filenames specified (" << filename << ", "
                << arg << ")" << std::endl;
            return 1;
        }
    }

    // retrieve code to run
    std::string code;
    if (filename == "") {
        std::string line;
        while (std::getline(std::cin, line) && line != "__END__") code += line;
    } else {
        std::ifstream infile(filename.c_str());
        if (infile.good()) {
            std::stringstream buf;
            buf << infile.rdbuf() << std::endl;
            code = buf.str();
        } else {
            std::cerr << "Could not read file " << filename << std::endl;
            return 1;
        }
    }

    // process -m (--minify) flag
    if (minify) {
        for (std::string s : Snowman::tokenize(code)) {
            std::cout << s;
        }
        return 0;
    }

    // run code
    sm.run(code);
}
