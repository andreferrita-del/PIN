#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "runtime/runtime.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static bool readFile(const fs::path& path, std::string& source) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    std::ostringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
    return true;
}

static void writeFile(const fs::path& path, const std::string& source) {
    std::ofstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot create file.");
    file << source;
}

static std::vector<pin::StmtPtr> parseSource(
    const std::string& source
) {
    pin::Lexer lexer(source);
    auto tokens = lexer.scan();

    for (const auto& token : tokens) {
        if (token.type == pin::TokenType::Invalid)
            throw std::runtime_error(
                "Invalid token at line " +
                std::to_string(token.line)
            );
    }

    pin::Parser parser(tokens);
    return parser.parse();
}

static std::string argument(const std::string& line) {
    size_t p = line.find(' ');
    if (p == std::string::npos) return "";
    return trim(line.substr(p + 1));
}

static void listFiles() {
    for (const auto& item : fs::directory_iterator(fs::current_path())) {
        std::cout << (item.is_directory() ? "[DIR]  " : "[FILE] ")
                  << item.path().filename().string() << '\n';
    }
}

static void editor(const fs::path& path) {
    std::string content;
    readFile(path, content);

    std::cout << "\nPIN Nano - " << path.string() << '\n';
    std::cout << "Commands: :w save | :q quit | :wq save and quit\n";
    std::cout << "----------------------------------------\n";

    if (!content.empty())
        std::cout << content;

    std::string line;

    while (true) {
        std::cout << "\nEDITOR> " << std::flush;

        if (!std::getline(std::cin, line))
            break;

        if (line == ":q")
            break;

        if (line == ":w" || line == ":wq") {
            writeFile(path, content);
            std::cout << "[OK] Saved " << path.string() << '\n';

            if (line == ":wq")
                break;

            continue;
        }

        content += line;
        content += '\n';
    }
}

static void help() {
    std::cout
        << "\nPIN Core 0.6 - Developer Console\n\n"
        << "FILES:\n"
        << "  pwd                 Current directory\n"
        << "  cd <folder>         Change directory\n"
        << "  ls                  List files\n"
        << "  mkdir <name>        Create directory\n"
        << "  create <file>       Create file\n"
        << "  nano <file>         Open PIN text editor\n\n"
        << "PIN:\n"
        << "  compile <file.pin>  Check PIN source\n"
        << "  run <file.pin>      Execute PIN source\n"
        << "  version             Show version\n"
        << "  clear               Clear screen\n"
        << "  help                Show commands\n"
        << "  exit                Close console\n\n";
}

static void repl() {
    pin::Runtime runtime;
    std::string line;

    std::cout
        << "\nPIN Core 0.6\n"
        << "A Compiler Made in C++\n"
        << "Developer Console\n"
        << "Type 'help' for commands.\n\n";

    while (true) {
        std::cout << "PIN> " << std::flush;

        if (!std::getline(std::cin, line))
            break;

        line = trim(line);
        if (line.empty()) continue;

        if (line == "exit" || line == "quit")
            break;

        if (line == "help") {
            help();
            continue;
        }

        if (line == "version") {
            std::cout << "PIN Core 0.6\n";
            continue;
        }

        if (line == "clear") {
            std::cout << "\033[2J\033[H" << std::flush;
            continue;
        }

        if (line == "pwd") {
            std::cout << fs::current_path().string() << '\n';
            continue;
        }

        if (line == "ls") {
            try {
                listFiles();
            } catch (const std::exception& e) {
                std::cerr << "[PIN ERROR] " << e.what() << '\n';
            }
            continue;
        }

        if (line.rfind("cd ", 0) == 0) {
            try {
                fs::current_path(argument(line));
                std::cout << fs::current_path().string() << '\n';
            } catch (const std::exception& e) {
                std::cerr << "[PIN ERROR] " << e.what() << '\n';
            }
            continue;
        }

        if (line.rfind("mkdir ", 0) == 0) {
            try {
                fs::create_directories(argument(line));
                std::cout << "[OK] Directory created.\n";
            } catch (const std::exception& e) {
                std::cerr << "[PIN ERROR] " << e.what() << '\n';
            }
            continue;
        }

        if (line.rfind("create ", 0) == 0) {
            fs::path path(argument(line));

            if (path.empty()) {
                std::cerr << "Usage: create file.pin\n";
                continue;
            }

            if (fs::exists(path)) {
                std::cerr << "[PIN ERROR] File already exists.\n";
                continue;
            }

            try {
                writeFile(path, "");
                std::cout << "[OK] Created " << path.string() << '\n';
            } catch (const std::exception& e) {
                std::cerr << "[PIN ERROR] " << e.what() << '\n';
            }
            continue;
        }

        if (line.rfind("nano ", 0) == 0) {
            fs::path path(argument(line));

            if (path.empty()) {
                std::cerr << "Usage: nano file.pin\n";
                continue;
            }

            try {
                editor(path);
            } catch (const std::exception& e) {
                std::cerr << "[PIN ERROR] " << e.what() << '\n';
            }
            continue;
        }

        if (line.rfind("compile ", 0) == 0) {
            std::string path = argument(line);
            std::string source;

            if (!readFile(path, source)) {
                std::cerr << "[PIN ERROR] Cannot open " << path << '\n';
                continue;
            }

            try {
                auto program = parseSource(source);
                (void)program;
                std::cout << "[OK] Syntax accepted: " << path << '\n';
                std::cout << "[PIN] Compilation check completed.\n";
            } catch (const std::exception& e) {
                std::cerr << "[PIN ERROR] " << e.what() << '\n';
            }
            continue;
        }

        if (line.rfind("run ", 0) == 0) {
            std::string path = argument(line);
            std::string source;

            if (!readFile(path, source)) {
                std::cerr << "[PIN ERROR] Cannot open " << path << '\n';
                continue;
            }

            try {
                auto program = parseSource(source);
                runtime.executeInteractive(program);
                std::cout << "[OK] Execution finished.\n";
            } catch (const std::exception& e) {
                std::cerr << "[PIN ERROR] " << e.what() << '\n';
            }
            continue;
        }

        try {
            auto program = parseSource(line);
            runtime.executeInteractive(program);
        } catch (const std::exception& e) {
            std::cerr << "[PIN ERROR] " << e.what() << '\n';
        }
    }

    std::cout << "\nPIN Console closed.\n";
}

int main(int argc, char** argv) {
    if (argc == 1 || std::string(argv[1]) == "repl") {
        repl();
        return 0;
    }

    std::string command = argv[1];

    if (command == "version") {
        std::cout << "PIN Core 0.6\n";
        return 0;
    }

    if (command == "help") {
        help();
        return 0;
    }

    if (argc < 3) {
        std::cerr << "Missing source file.\n";
        return 1;
    }

    std::string source;

    if (!readFile(argv[2], source)) {
        std::cerr << "Cannot open source file.\n";
        return 1;
    }

    try {
        auto program = parseSource(source);

        if (command == "compile" || command == "check") {
            std::cout << "[OK] Syntax accepted.\n";
            return 0;
        }

        if (command == "run") {
            pin::Runtime runtime;
            runtime.execute(program);
            return 0;
        }

        std::cerr << "Unknown command: " << command << '\n';
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "[PIN ERROR] " << e.what() << '\n';
        return 1;
    }
}
