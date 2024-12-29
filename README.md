# VirtualMachine
> Stack-based virtual machine. Version 1.0.6.

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
0x0E | 3 | 0 | jg
0x0F | 3 | 0 | je
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
0xF1 | 1 | 0 | jmp
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
$ hexdump main.bcd
0000000 000a 0000 0a0a 0000 1200 0a1c 0000 1100
0000010 1df1 ff0a ffff 1bfe ff0a ffff 1bff 000a
0000020 0000 0a02 0000 6000 0aa2 ffff ffff 0a1b
0000030 0000 0100 0ab0 ffff ffff ff0a ffff 1afe
0000040 0a0b ffff fdff 0a1b ffff feff c01b ff0a
0000050 ffff 0aff ffff fcff 0b1a 000a 0000 f118
0000060 f10b                                   
0000062
```
