#include "parser.hpp"
#include "printer.hpp"
#include <iostream>
#include <fstream>
#include <vector>

static void usage(const char* argv0) {
    std::cerr << "usage: " << argv0 << " [options] <file.luac | ->\n"
              << "  --no-color   disable ansi colors\n"
              << "  --no-hex     hide hex column\n"
              << "  --no-lines   hide line numbers\n"
              << "  --no-locals  hide local/upvalue tables\n"
              << "  --no-consts  hide constants table\n"
              << "  --no-protos  hide nested protos\n"
              << "  --compact    suppress inline hints\n";
}

static std::vector<uint8_t> readfile(const std::string& path) {
    std::istream* src;
    std::ifstream f;
    if (path == "-") {
        src = &std::cin;
        std::cin.sync_with_stdio(false);
    } else {
        f.open(path, std::ios::binary);
        if (!f.is_open()) throw std::runtime_error("can't open: " + path);
        src = &f;
    }
    return {std::istreambuf_iterator<char>(*src), {}};
}

int main(int argc, char* argv[]) {
    PrintOpts opts;
    std::string path;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if      (a == "--no-color")  opts.color   = false;
        else if (a == "--no-hex")    opts.hex     = false;
        else if (a == "--no-lines")  opts.lines   = false;
        else if (a == "--no-locals") opts.locals  = false;
        else if (a == "--no-consts") opts.consts  = false;
        else if (a == "--no-protos") opts.protos  = false;
        else if (a == "--compact")   opts.compact = true;
        else if (a == "--help")      { usage(argv[0]); return 0; }
        else if (a[0] == '-' && a != "-") { std::cerr << "unknown: " << a << "\n"; return 1; }
        else if (!path.empty())      { std::cerr << "too many files\n"; return 1; }
        else path = a;
    }

    if (path.empty()) { usage(argv[0]); return 1; }

    try {
        auto data = readfile(path);
        auto proto = Parser(std::move(data)).parse();
        Printer(std::cout, opts).print(proto);
    } catch (const ParseError& e) {
        std::cerr << "parse error: " << e.what() << "\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
