# VirtualMachine
> Stack-based virtual machine. Version 1.0.0.

### Pseudo instructions (num = 3)
1. ""
2. "labl"
3. ";"

### Main instructions (num = 12)
1.  [0x0A] "push"  
2.  [0x0B] "pop"   
3.  [0x0C] "add"   
4.  [0x0D] "sub"   
5.  [0x0E] "jl"    
6.  [0x0F] "jg"    
7.  [0x1A] "je"    
8.  [0x1B] "stor" 
9.  [0x1C] "load"  
10. [0x1D] "call"  
11. [0x1E] "ret"   
12. [0x1F] "hlt"   

### Additional instructions (num = 14)
1.  [0xA0] "mul"  
2.  [0xB0] "div"   
3.  [0xC0] "mod"   
4.  [0xD0] "shr"   
5.  [0xE0] "shl"    
6.  [0xF0] "xor"    
7.  [0xA1] "and"    
8.  [0xB1] "or" 
9.  [0xC1] "not"  
10. [0xD1] "jmp"  
11. [0xE1] "jne"   
12. [0xF1] "jle"   
13. [0xA2] "jge"   
14. [0xB2] "allc"   

### Interface functions
```c
extern int cvm_compile(FILE *output, FILE *input);
extern int cvm_load(uint8_t *memory, int32_t msize);
extern int cvm_run(int32_t **output, int32_t *input);
```

### Compile and run
```
$ make 
> [ 3 24 19 ]
```

### Example: caesar encryption
```asm
labl begin
    ; A[3] <- (10, 15, 20)
    push 10
    push 15
    push 20
    ; K <- 9
    push 9
    ; S <- size(A)
    push 3
    push caesar
    call
    push end
    jmp
labl end
    pop
    pop
    hlt

labl caesar
    ; I = 0
    push 0
labl caesar_iter
    ; IF I >= S
    push -1
    load
    push -4
    load
    push caesar_exit
    jge

    ; A' <- (K + A[I]) mod 26
    push -4
    load
    push -6
    push -3
    load
    sub
    load
    add
    push 26
    mod

    ; A[I] <- A'
    push -1
    push -6
    push -4
    load
    sub
    stor
    pop

    ; I <- I + 1
    push -1
    load
    push 1
    add
    push -1
    push -2
    stor
    pop

    push caesar_iter
    jmp 
labl caesar_exit
    pop
    ret
```

### Output .File main.vme
```
0a00 0000 0a0a 0000 000f 0a00 0000 140a
0000 0009 0a00 0000 030a 0000 0028 1d0a
0000 0025 d10b 0b1f 0a00 0000 000a ffff
ffff 1c0a ffff fffc 1c0a 0000 008a a20a
ffff fffc 1c0a ffff fffa 0aff ffff fd1c
0d1c 0c0a 0000 001a c00a ffff ffff 0aff
ffff fa0a ffff fffc 1c0d 1b0b 0aff ffff
ff1c 0a00 0000 010c 0aff ffff ff0a ffff
fffe 1b0b 0a00 0000 2dd1 0b1e 
```
