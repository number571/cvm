# VirtualMachine
> Stack-based virtual machine. Version 1.0.4.

### Pseudo instructions (num = 2)
1. [0x11] ";"
2. [0x22] "labl"

### Null instructions (num = 2)
1. [0xAA] "\0" (void string)
2. [0xBB] "\1" (undefined code)

### Main instructions (num = 11)
1.  [0x0A] "push"  
2.  [0x0B] "pop"   
3.  [0x0C] "inc"   
4.  [0x0D] "dec"   
5.  [0x0E] "jg"    
6.  [0x0F] "je"    
7.  [0x1A] "jmp"    
8.  [0x1B] "stor" 
9.  [0x1C] "load"  
10. [0x1D] "call"  
11. [0x1E] "hlt"   

### Additional instructions (num = 16)
1.  [0xA0] "add"  
2.  [0xB0] "sub"   
3.  [0xC0] "mul"   
4.  [0xD0] "div"   
5.  [0xE0] "mod"    
6.  [0xF0] "shr"    
7.  [0xA1] "shl"    
8.  [0xB1] "xor" 
9.  [0xC1] "and"  
10. [0xD1] "or"  
11. [0xE1] "not"   
12. [0xF1] "jl"   
13. [0xA2] "jne"   
14. [0xB2] "jle"   
15. [0xC2] "jge"   
16. [0xD2] "allc"   

### Interface functions
```c
extern int cvm_compile(FILE *output, FILE *input);
extern int cvm_load(uint8_t *memory, int32_t msize);
extern int cvm_run(int32_t **output, int32_t *input);
```

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
