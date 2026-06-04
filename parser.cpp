#include "parser.hpp"
#include <cstring>
#include <sstream>

static constexpr uint8_t SIG[4] = {0x1B, 'L', 'u', 'a'};

Parser::Parser(std::vector<uint8_t> d)
    : buf(std::move(d)), pos(0), le(true), isz(4), ssz(8), insz(4), numsz(8), numint(false) {}

template<typename T>
T Parser::readle(int n) {
    if (pos + n > buf.size())
        throw ParseError("unexpected eof at " + std::to_string(pos));
    T v = 0;
    for (int i = 0; i < n; i++)
        v |= le ? (T(buf[pos+i]) << (8*i)) : (T(buf[pos+i]) << (8*(n-1-i)));
    pos += n;
    return v;
}

uint8_t Parser::byte() {
    if (pos >= buf.size()) throw ParseError("unexpected eof");
    return buf[pos++];
}

int32_t Parser::readint()  { return readle<int32_t>(isz); }
size_t  Parser::readsz()   { return readle<size_t>(ssz); }

double Parser::readnum() {
    if (numsz == 8) {
        uint64_t b = readle<uint64_t>(8);
        double v; std::memcpy(&v, &b, 8); return v;
    }
    uint32_t b = readle<uint32_t>(4);
    float v; std::memcpy(&v, &b, 4); return v;
}

std::string Parser::readstr() {
    size_t len = readsz();
    if (!len) return "";
    if (pos + len > buf.size()) throw ParseError("string oob at " + std::to_string(pos));
    std::string s(reinterpret_cast<const char*>(&buf[pos]), len - 1);
    pos += len;
    return s;
}

Ins Parser::readins() {
    uint32_t r = readle<uint32_t>(insz);
    Ins i;
    i.raw = r;
    i.op  = r & 0x3F;
    i.A   = (r >> 6)  & 0xFF;
    i.C   = (r >> 14) & 0x1FF;
    i.B   = (r >> 23) & 0x1FF;
    i.Bx  = (r >> 14) & 0x3FFFF;
    i.sBx = i.Bx - 131071;
    return i;
}

Constant Parser::readconst() {
    uint8_t t = byte();
    switch (t) {
        case 0: return std::monostate{};
        case 1: return bool(byte());
        case 3: return readnum();
        case 4: return readstr();
        default: throw ParseError("unknown const type " + std::to_string(t));
    }
}

std::shared_ptr<Proto> Parser::readproto() {
    auto p = std::make_shared<Proto>();
    p->source      = readstr();
    p->linedef     = readint();
    p->lastlinedef = readint();
    p->nupvals     = byte();
    p->nparams     = byte();
    p->vararg      = byte();
    p->maxstack    = byte();

    int n = readint();
    p->code.reserve(n);
    for (int i = 0; i < n; i++) p->code.push_back(readins());

    n = readint();
    p->consts.reserve(n);
    for (int i = 0; i < n; i++) p->consts.push_back(readconst());

    n = readint();
    p->protos.reserve(n);
    for (int i = 0; i < n; i++) p->protos.push_back(readproto());

    n = readint();
    p->lines.resize(n);
    for (int i = 0; i < n; i++) p->lines[i] = readint();

    n = readint();
    p->locals.reserve(n);
    for (int i = 0; i < n; i++) {
        LocalVar lv;
        lv.name  = readstr();
        lv.start = readint();
        lv.end   = readint();
        p->locals.push_back(std::move(lv));
    }

    n = readint();
    p->upvalnames.reserve(n);
    for (int i = 0; i < n; i++) p->upvalnames.push_back(readstr());

    return p;
}

void Parser::header() {
    if (buf.size() < 12) throw ParseError("file too small");
    for (int i = 0; i < 4; i++)
        if (byte() != SIG[i]) throw ParseError("bad lua signature");
    uint8_t ver = byte();
    if (ver != 0x51) {
        std::ostringstream ss;
        ss << std::hex << int(ver);
        throw ParseError("unsupported version 0x" + ss.str());
    }
    if (byte() != 0x00) throw ParseError("non-standard format");
    le     = byte() == 1;
    isz    = byte();
    ssz    = byte();
    insz   = byte();
    numsz  = byte();
    numint = byte() == 1;
}

std::shared_ptr<Proto> Parser::parse() {
    header();
    return readproto();
}
