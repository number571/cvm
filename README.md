# VirtualMachine
> Stack-based virtual machine. Version 1.0.4.

### Pseudo instructions
Code | Instruction
:---: | :---: |
0xAA | ";" (comment)
0xBB | "labl" (label)

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
0x1A | 1 | 0 | jmp
0x1B | 2 | 0 | stor
0x1C | 1 | 0 | load
0x1D | 1 | 0 | call
0x1E | 0 | 0 | hlt

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
0xF1 | 3 | 0 | jl
0xA2 | 3 | 0 | jne
0xB2 | 3 | 0 | jle
0xC2 | 3 | 0 | jge
0xD2 | 1 | 0 | allc

### Compile and run
```
$ make install
> git clone https://github.com/number571/extclib.git
$ make 
> gcc -o cvm -Wall -std=c99 cvm.c cvmkernel.c extclib/type/stack.c extclib/type/hashtab.c extclib/type/list.c 
> ./cvm build main.vms -o main.vme
> ./cvm run main.vme
> 
{
	"result": [50],
	"return": 0
}
```

### Example: mul5 function (assembly code)
```asm
labl _start
	push begin 
	jmp

; main
labl begin
	; mul5(x) = x * 5
	; where x = 10
	push 10
	push mul5
	call
	push end
	jmp	

; exit 
labl end
	hlt

; x = arg[1]
labl mul5
	; y = x * 5
	push -2
	load 
	push 5
	mul

	; x = y
	push -1
	push -3
	stor 

	; return
	pop
	jmp
```

### Example: mul5 function (binary code)
```
0a00 0000 061a 0a00 0000 0a0a 0000 0018
1d0a 0000 0017 1a1e 0aff ffff fe1c 0a00
0000 05c0 0aff ffff ff0a ffff fffd 1b0b
1a
```
