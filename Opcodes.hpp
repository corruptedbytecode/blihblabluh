#pragma once
#include <string>

enum class OpMode { ABC, ABx, AsBx };

struct OpInfo {
    std::string name;
    OpMode mode;
    bool a, b, c;
};

static const OpInfo OPS[38] = {
    {"MOVE",     OpMode::ABC,  true,  true,  false},
    {"LOADK",    OpMode::ABx,  true,  true,  false},
    {"LOADBOOL", OpMode::ABC,  true,  true,  true },
    {"LOADNIL",  OpMode::ABC,  true,  true,  false},
    {"GETUPVAL", OpMode::ABC,  true,  true,  false},
    {"GETGLOBAL",OpMode::ABx,  true,  true,  false},
    {"GETTABLE", OpMode::ABC,  true,  true,  true },
    {"SETGLOBAL",OpMode::ABx,  true,  true,  false},
    {"SETUPVAL", OpMode::ABC,  true,  true,  false},
    {"SETTABLE", OpMode::ABC,  true,  true,  true },
    {"NEWTABLE", OpMode::ABC,  true,  true,  true },
    {"SELF",     OpMode::ABC,  true,  true,  true },
    {"ADD",      OpMode::ABC,  true,  true,  true },
    {"SUB",      OpMode::ABC,  true,  true,  true },
    {"MUL",      OpMode::ABC,  true,  true,  true },
    {"DIV",      OpMode::ABC,  true,  true,  true },
    {"MOD",      OpMode::ABC,  true,  true,  true },
    {"POW",      OpMode::ABC,  true,  true,  true },
    {"UNM",      OpMode::ABC,  true,  true,  false},
    {"NOT",      OpMode::ABC,  true,  true,  false},
    {"LEN",      OpMode::ABC,  true,  true,  false},
    {"CONCAT",   OpMode::ABC,  true,  true,  true },
    {"JMP",      OpMode::AsBx, false, true,  false},
    {"EQ",       OpMode::ABC,  true,  true,  true },
    {"LT",       OpMode::ABC,  true,  true,  true },
    {"LE",       OpMode::ABC,  true,  true,  true },
    {"TEST",     OpMode::ABC,  true,  false, true },
    {"TESTSET",  OpMode::ABC,  true,  true,  true },
    {"CALL",     OpMode::ABC,  true,  true,  true },
    {"TAILCALL", OpMode::ABC,  true,  true,  true },
    {"RETURN",   OpMode::ABC,  true,  true,  false},
    {"FORLOOP",  OpMode::AsBx, true,  true,  false},
    {"FORPREP",  OpMode::AsBx, true,  true,  false},
    {"TFORLOOP", OpMode::ABC,  true,  false, true },
    {"SETLIST",  OpMode::ABC,  true,  true,  true },
    {"CLOSE",    OpMode::ABC,  true,  false, false},
    {"CLOSURE",  OpMode::ABx,  true,  true,  false},
    {"VARARG",   OpMode::ABC,  true,  true,  false},
};

inline const OpInfo* opinfo(uint8_t op) {
    return op < 38 ? &OPS[op] : nullptr;
}
