<center>
    <h1>ASTRISC-16-V2</h1>
</center>

The second version of [ASTRISC-16](https://github.com/astrosLab/ASTRISC-16), with an improved instruction set, more accurate emulated hardware and a plugin system for easily adding libraries and interfaces.
Planned features are an emulator, assembler, compiler, and a few plugins.

## Specifications 
#### Features
- 16 bit data/address bus
- 64KiB RAM, each address stores 8 bits
- Shared data/program RAM
- 16 memory-mapped I/O ports
- Reserved RAM area for special registers
- Flags: zero, signed, carry, overflow (ALU updates carry, CMP updates others)
- 32 Opcodes
- Micro ops
- 8 general registers
- Pointer support
- Relative branching
- Interrupts

#### Reserved RAM Addresses
- 0x???? Program counter
- 0x???? Error code
- 0x???? Flags 
- 0x???? Interrupt status
- 0x???? Stack pointer 
- 0x???? Call stack pointer
- 0xFFEF - 0xFBEF Stack region
- 0xFBEF - 0xF7EF Call stack region
- 0xFFF0 - 0xFFFF General I/O port region

#### Error codes
Instead of halting when recieving an error, the error code value in RAM gets updated.
| Name | Code |
|------|------|
|None|0x0|
|Stack overflow|0x1|
|Stack underflow|0x2|
|Call stack overflow|0x3|
|Call stack overflow|0x4|
|Division by 0|0x5|

## ASTRISC ISA

#### Keyword Lengths
| Name | Bit length |
|------|------------|
|OPCODE| 5 |
|Any REG| 3 |
|IMM HIGH/LOW| 16 |

### Opcodes
| Hex Code | Name | Byte 1 | Byte 2 | Byte 3 | Description      |
|----------|------|--------|--------|--------|------------------|
|0x00|NOP|OPCODE|||No operation|
|0x01|LDI|OPCODE, REG|IMM_HIGH|IMM_LOW|Load the immediate into a register|
|0x02|LOD|OPCODE, DST_REG|ADDR_REG||Load into DST_REG using address in ADDR_REG|
|0x03|STR|OPCODE, SRC_REG|ADDR_REG||Store from SRC_REG using address in ADDR_REG|
|0x04|CALL|OPCODE|ADDR_HIGH|ADDR_LOW|Push next instruction's address to call stack, jump to ADDR|
|0x05|RET|OPCODE|||Pop call stack, jump to returned address|
|0x06|PUSH|OPCODE, REG|||Pushes value in REG onto stack|
|0x07|POP|OPCODE, REG|||Pops stack, store into REG|
|0x08|MOV|OPCODE, SRC_REG|DST_REG||Copy value from SRC_REG into DST_REG|
|0x09|ADD|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG + SRC2_REG -> DST_REG|
|0x0A|SUB|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG - SRC2_REG -> DST_REG|
|0x0B|MUL|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG * SRC2_REG -> DST_REG|
|0x0C|DIV|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG / SRC2_REG -> DST_REG|
|0x0D|MOD|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG % SRC2_REG -> DST_REG|
|0x0E|INC|OPCODE, SRC1_REG|DST_REG||SRC1_REG + 1 -> DST_REG|
|0x0F|DEC|OPCODE, SRC1_REG|DST_REG||SRC1_REG - 1 -> DST_REG|
|0x10|SHL|OPCODE, SRC1_REG|DST_REG, IMM_4_BIT||SRC1_REG << IMM_4_BIT -> DST_REG|
|0x11|SHR|OPCODE, SRC1_REG|DST_REG, IMM_4_BIT||SRC1_REG >> IMM_4_BIT -> DST_REG|
|0x12|AND|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG & SRC2_REG -> DST_REG|
|0x13|NAND|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG ~& SRC2_REG -> DST_REG|
|0x14|OR |OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG \| SRC2_REG -> DST_REG|
|0x15|NOR|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG ~\| SRC2_REG -> DST_REG|
|0x16|XOR|OPCODE, SRC1_REG|SRC2_REG, DST_REG||SRC1_REG ^ SRC2_REG -> DST_REG|
|0x17|CMP|OPCODE, SRC1_REG|SRC2_REG||Update flags (except carry flags) based on SRC1_REG - SRC2_REG|
|0x18|JMP|OPCODE|ADDR_HIGH|ADDR_LOW|Jump to relative address|
|0x19|BZ|OPCODE|ADDR_HIGH|ADDR_LOW|Jump to relative address if ZERO|
|0x1A|BS|OPCODE|ADDR_HIGH|ADDR_LOW|Jump to relative address if SIGNED|
|0x1B|BC|OPCODE|ADDR_HIGH|ADDR_LOW|Jump to relative address if CARRY|
|0x1C|BO|OPCODE|ADDR_HIGH|ADDR_LOW|Jump to relative address if OVERFLOW|
|0x1D|NONE||||Reserved, same as NOP|
|0x1E|NONE||||Reserved, same as NOP|
|0x1F|HALT||||Stops the clock|

### Assembler Pseudoinstructions
These instructions are replaced by multiple opcodes when assembled.
| Name | Parameters | Expansion | Description |
|------|------------|-----------|-------------|
|BNZ|ADDR|BZ 2<br>JMP ADDR| Offset PC by ADDR if not ZERO |
|BNS|ADDR|BS 2<br>JMP ADDR| Offset PC by ADDR if not SIGNED |
|BNC|ADDR|BC 2<br>JMP ADDR| Offset PC by ADDR if not CARRY |
|BNO|ADDR|BO 2<br>JMP ADDR| Offset PC by ADDR if not OVERFLOW |

### Micro Ops
WIP

