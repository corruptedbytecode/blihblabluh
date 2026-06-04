#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <stdexcept>

struct LocalVar {
    std::string name;
    int32_t start, end;
};

using Constant = std::variant<std::monostate, bool, double, std::string>;

struct Ins {
    uint32_t raw;
    uint8_t op;
    int32_t A, B, C, Bx, sBx;
};

struct Proto {
    std::string source;
    int32_t linedef, lastlinedef;
    uint8_t nupvals, nparams, vararg, maxstack;
    std::vector<Ins> code;
    std::vector<Constant> consts;
    std::vector<std::shared_ptr<Proto>> protos;
    std::vector<int32_t> lines;
    std::vector<LocalVar> locals;
    std::vector<std::string> upvalnames;
};

struct ParseError : std::runtime_error {
    explicit ParseError(const std::string& m) : std::runtime_error(m) {}
};

class Parser {
public:
    explicit Parser(std::vector<uint8_t> d);
    std::shared_ptr<Proto> parse();

private:
    std::vector<uint8_t> buf;
    size_t pos;
    bool le;
    int isz, ssz, insz, numsz;
    bool numint;

    uint8_t byte();
    int32_t readint();
    size_t readsz();
    double readnum();
    std::string readstr();
    Ins readins();
    Constant readconst();
    std::shared_ptr<Proto> readproto();
    void header();

    template<typename T> T readle(int n);
};
