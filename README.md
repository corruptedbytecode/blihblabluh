# luaprint

cli tool that parses lua 5.1 bytecode and prints it in a readable format. useful if you want to inspect what luac actually outputs without reading raw hex.

works on `.luac` files or stdin. no dependencies, just c++17.

## build

```
make
```

## usage

```
./luaprint script.luac
./luaprint - < script.luac
```

flags:

```
--no-color    no ansi colors
--no-hex      hide the hex column
--no-lines    hide source line numbers
--no-locals   hide local variable table
--no-consts   hide constants table
--no-protos   hide nested function protos
--compact     hide inline hints on call/return
```

## output

for each proto you get:

- header (params, upvalues, stack size, source file)
- constants table with resolved values
- locals with their pc ranges
- upvalue names if any
- full instruction listing with resolved operand references

nested functions are printed recursively with indentation.

constants are resolved inline so instead of `Bx=3` you see `Bx=3="print"`. rk operands on arithmetic instructions show whether they reference a register or constant.

example:

```
/// Proto ///
source:      @Input.lua
params:      0
instructions:25
constants:   8
protos:      1

-- constants --
[0]  10
[1]  20
[3]  "print"

-- code --
line  idx   [hex]           opcode      operands
------------------------------------------------------------------------
5        1  [0x00000041]  LOADK         A=1  Bx=0=10
7        6  [0x018080DC]  CALL          A=3  B=3  C=2  ; args=2 ret=1
9        7  [0x8100C018]  LT            A=0  B=258  C=3
```

## lua 5.1

the binary format this parses is lua 5.1 specifically. luau (roblox) is based on 5.1 so it mostly works there too, though luau has diverged in some areas.

header validation checks the magic bytes, version byte (`0x51`), and format byte. endianness, int size, and number size are all read from the header so it handles non-standard builds fine.

## notes

opcode table covers all 38 standard lua 5.1 opcodes. sBx is decoded as `Bx - 131071` per spec. FORLOOP/FORPREP jump targets are shown as raw sBx offsets.
