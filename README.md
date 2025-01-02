# VirtualMachine
> Stack-based virtual machine. Version 1.0.9

### Pseudo instructions
Code | Instruction
:---: | :---: |
0x11 | ";" (comment)
0x22 | "labl" (label)

### Null instructions
Code | Instruction
:---: | :---: |
0xAA | (void string)
0xBB | (undefined code)

### Main instructions
Bytecode | Stack | Args | Instruction
:---: | :---: | :---: | :---: |
0x0A | 0 | 1 | push
0x0B | 1 | 0 | pop
0x0C | 1 | 0 | inc
0x0D | 1 | 0 | dec
0x0E | 1 | 0 | jmp
0x0F | 3 | 0 | jg
0x1A | 2 | 0 | stor
0x1B | 1 | 0 | load
0x1C | 1 | 0 | call
0x1D | 0 | 0 | hlt

### Interface functions
```c
extern int cvm_compile(FILE *output, FILE *input);
extern int cvm_load(uint8_t *memory, int32_t msize);
extern int cvm_run(int32_t **output, int32_t *input);
```

### Additional instructions
Bytecode | Stack | Args | Instruction
:---: | :---: | :---: | :---: |
0xA0 | 2 | 0 | add
0xB0 | 2 | 0 | sub
0xC0 | 2 | 0 | mul
0xD0 | 2 | 0 | div
0xE0 | 2 | 0 | mod
0xF0 | 2 | 0 | shr
0xA1 | 2 | 0 | shl
0xB1 | 2 | 0 | xor
0xC1 | 2 | 0 | and
0xD1 | 2 | 0 | or
0xE1 | 1 | 0 | not
0xF1 | 3 | 0 | je
0xA2 | 3 | 0 | jl
0xB2 | 3 | 0 | jne
0xC2 | 3 | 0 | jle
0xD2 | 3 | 0 | jge
0xE2 | 1 | 0 | allc

### Compile and run
```bash
$ cp examples/fact10.asm main.asm
$ make
{
	"result": [3628800],
	"return": 0
}
```

```bash
$ hexdump --format '16/1 "%02X " "\n"' main.bcd
0A 00 00 00 0A 0A 00 00 00 12 1C 0A 00 00 00 11
0E 1D 0A FF FF FF FE 1B 0A 00 00 00 02 0A FF FF
FF FE 1B 0A 00 00 00 5B 0F 0A FF FF FF FF 1B 0D
0A FF FF FF FF 0A FF FF FF FE 1A 0B 0A FF FF FF
FD 1B 0A FF FF FF FE 1B C0 0A FF FF FF FF 0A FF
FF FF FC 1A 0B 0A 00 00 00 18 0E 0B 0E
```
