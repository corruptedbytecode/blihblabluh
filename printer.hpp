#pragma once
#include "parser.hpp"
#include <ostream>

struct PrintOpts {
    bool color    = true;
    bool hex      = true;
    bool lines    = true;
    bool locals   = true;
    bool consts   = true;
    bool protos   = true;
    bool compact  = false;
};

class Printer {
public:
    Printer(std::ostream& out, PrintOpts opts = {});
    void print(const std::shared_ptr<Proto>& p, int depth = 0);

private:
    std::ostream& out;
    PrintOpts opts;

    void pheader(const Proto& p, int d);
    void pconsts(const Proto& p, int d);
    void plocals(const Proto& p, int d);
    void pcode(const Proto& p, int d);
    void pins(const Proto& p, const Ins& ins, int idx, int d);

    std::string fmtconst(const Constant& c);
    std::string fmtops(const Proto& p, const Ins& ins);
    std::string rkref(const Proto& p, int idx);
    std::string col(const std::string& s, const char* code);
    std::string ind(int d);
};
