#include "printer.hpp"
#include "opcodes.hpp"
#include <iomanip>
#include <sstream>

#define RST  "\033[0m"
#define COP  "\033[1;36m"
#define CREG "\033[1;33m"
#define CCN  "\033[1;32m"
#define CLB  "\033[1;35m"
#define CHD  "\033[1;37m"
#define CDM  "\033[2;37m"
#define CNUM "\033[1;34m"
#define CSTR "\033[1;31m"

Printer::Printer(std::ostream& o, PrintOpts opts) : out(o), opts(opts) {}

std::string Printer::col(const std::string& s, const char* c) {
    return opts.color ? (c + s + RST) : s;
}

std::string Printer::ind(int d) { return std::string(d * 4, ' '); }

std::string Printer::fmtconst(const Constant& c) {
    return std::visit([&](auto&& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::monostate>) return col("nil", CDM);
        else if constexpr (std::is_same_v<T, bool>)      return col(v ? "true" : "false", CNUM);
        else if constexpr (std::is_same_v<T, double>) {
            std::ostringstream ss;
            ss << std::setprecision(14) << v;
            return col(ss.str(), CNUM);
        } else {
            std::string e;
            for (char ch : v) {
                if      (ch == '"')  e += "\\\"";
                else if (ch == '\\') e += "\\\\";
                else if (ch == '\n') e += "\\n";
                else if (ch == '\r') e += "\\r";
                else if (ch == '\0') e += "\\0";
                else e += ch;
            }
            return col("\"" + e + "\"", CSTR);
        }
    }, c);
}

std::string Printer::rkref(const Proto& p, int idx) {
    if (idx >= 256) {
        int k = idx - 256;
        std::string base = col("K(" + std::to_string(k) + ")", CCN);
        if (k < (int)p.consts.size())
            base += col("=" + fmtconst(p.consts[k]), CDM);
        return base;
    }
    return col("R(" + std::to_string(idx) + ")", CREG);
}

std::string Printer::fmtops(const Proto& p, const Ins& ins) {
    auto* info = opinfo(ins.op);
    if (!info) return "";
    std::ostringstream ss;

    switch (info->mode) {
        case OpMode::ABC:
            ss << col("A=" + std::to_string(ins.A), CREG);
            if (info->b) {
                ss << "  ";
                ss << ((ins.op >= 12 && ins.op <= 17)
                    ? "B=" + rkref(p, ins.B)
                    : col("B=" + std::to_string(ins.B), CREG));
            }
            if (info->c) {
                ss << "  ";
                ss << ((ins.op >= 12 && ins.op <= 17)
                    ? "C=" + rkref(p, ins.C)
                    : col("C=" + std::to_string(ins.C), CREG));
            }
            break;
        case OpMode::ABx:
            ss << col("A=" + std::to_string(ins.A), CREG) << "  ";
            if (ins.op == 1 || ins.op == 5 || ins.op == 7) {
                ss << col("Bx=" + std::to_string(ins.Bx), CCN);
                if (ins.Bx < (int)p.consts.size())
                    ss << col("=" + fmtconst(p.consts[ins.Bx]), CDM);
            } else {
                ss << col("Bx=" + std::to_string(ins.Bx), CCN);
            }
            break;
        case OpMode::AsBx:
            if (info->a) ss << col("A=" + std::to_string(ins.A), CREG) << "  ";
            ss << col("sBx=" + std::to_string(ins.sBx), CLB);
            if (ins.op == 22)
                ss << col("  ; -> " + std::to_string(ins.sBx), CDM);
            break;
    }
    return ss.str();
}

void Printer::pins(const Proto& p, const Ins& ins, int idx, int d) {
    auto* info = opinfo(ins.op);
    std::string opname = info ? info->name : ("OP_" + std::to_string(ins.op));
    int line = idx < (int)p.lines.size() ? p.lines[idx] : -1;

    out << ind(d);

    if (opts.lines) {
        std::string ls = line > 0 ? std::to_string(line) : "?";
        out << col(ls, CDM) << std::string(6 - ls.size(), ' ');
    }

    out << std::setw(4) << idx << "  ";

    if (opts.hex) {
        std::ostringstream hx;
        hx << "[0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << ins.raw << "]  ";
        out << col(hx.str(), CDM);
    }

    std::ostringstream op;
    op << std::left << std::setw(12) << opname;
    out << col(op.str(), COP) << "  " << fmtops(p, ins);

    if (!opts.compact) {
        if (ins.op == 28 || ins.op == 29) {
            auto fmt = [](int v) { return v == 0 ? std::string("var") : std::to_string(v - 1); };
            out << col("  ; args=" + fmt(ins.B) + " ret=" + fmt(ins.C), CDM);
        } else if (ins.op == 30) {
            out << col("  ; ret=" + (ins.B == 0 ? std::string("var") : std::to_string(ins.B - 1)), CDM);
        }
    }
    out << "\n";
}

void Printer::pheader(const Proto& p, int d) {
    auto kv = [&](const std::string& k, const std::string& v) {
        out << ind(d) << col(k, CDM) << v << "\n";
    };
    out << ind(d) << col("/// Proto ///", CHD) << "\n";
    kv("source:      ", p.source.empty() ? col("(unknown)", CDM) : p.source);
    kv("defined:     ", std::to_string(p.linedef) + " - " + std::to_string(p.lastlinedef));
    kv("params:      ", std::to_string(p.nparams));
    kv("upvalues:    ", std::to_string(p.nupvals));
    kv("vararg:      ", std::to_string(p.vararg));
    kv("maxstack:    ", std::to_string(p.maxstack));
    kv("instructions:", std::to_string(p.code.size()));
    kv("constants:   ", std::to_string(p.consts.size()));
    kv("protos:      ", std::to_string(p.protos.size()));
    out << "\n";
}

void Printer::pconsts(const Proto& p, int d) {
    if (p.consts.empty()) return;
    out << ind(d) << col("-- constants --", CHD) << "\n";
    for (size_t i = 0; i < p.consts.size(); i++)
        out << ind(d) << col("[" + std::to_string(i) + "]", CCN) << "  " << fmtconst(p.consts[i]) << "\n";
    out << "\n";
}

void Printer::plocals(const Proto& p, int d) {
    if (p.locals.empty() && p.upvalnames.empty()) return;
    if (!p.locals.empty()) {
        out << ind(d) << col("-- locals --", CHD) << "\n";
        for (size_t i = 0; i < p.locals.size(); i++) {
            const auto& lv = p.locals[i];
            out << ind(d) << col("[" + std::to_string(i) + "]", CREG)
                << "  " << lv.name
                << col("  pc=" + std::to_string(lv.start) + ".." + std::to_string(lv.end), CDM) << "\n";
        }
        out << "\n";
    }
    if (!p.upvalnames.empty()) {
        out << ind(d) << col("-- upvalues --", CHD) << "\n";
        for (size_t i = 0; i < p.upvalnames.size(); i++)
            out << ind(d) << col("[" + std::to_string(i) + "]", CLB) << "  " << p.upvalnames[i] << "\n";
        out << "\n";
    }
}

void Printer::pcode(const Proto& p, int d) {
    out << ind(d) << col("-- code --", CHD) << "\n";
    out << ind(d);
    if (opts.lines) out << col("line  ", CDM);
    out << col("idx   ", CDM);
    if (opts.hex)   out << col("[hex]           ", CDM);
    out << col("opcode      operands\n", CDM);
    out << ind(d) << std::string(opts.hex ? 72 : 54, '-') << "\n";
    for (size_t i = 0; i < p.code.size(); i++)
        pins(p, p.code[i], i, d);
    out << "\n";
}

void Printer::print(const std::shared_ptr<Proto>& p, int depth) {
    if (!p) return;
    pheader(*p, depth);
    if (opts.consts) pconsts(*p, depth);
    if (opts.locals) plocals(*p, depth);
    pcode(*p, depth);
    if (opts.protos) {
        for (size_t i = 0; i < p->protos.size(); i++) {
            out << ind(depth) << col("-- nested proto [" + std::to_string(i) + "] --", CLB) << "\n\n";
            print(p->protos[i], depth + 1);
        }
    }
}
