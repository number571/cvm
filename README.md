# VirtualMachine
> Stack-based virtual machine.

### Instructions (num = 14)
1.  [0x00] "push"  
2.  [0x01] "pop"   
3.  [0x02] "add"   
4.  [0x03] "sub"   
5.  [0x04] "jmp"   
6.  [0x05] "jl"    
7.  [0x06] "jg"    
8.  [0x07] "jr"  
9.  [0x08] "store" 
10. [0x09] "load"  
11. [0x0A] "call"  
12. [0x0B] "ret"   
13. [0x0C] "hlt"   
14. [0x0D] "alloc" 

### Pseudo instructions (num = 2)
1. "label"
2. ";"

### Interface functions
```c
// translate source file (input) into executable (output)
extern int readvm_src(FILE *output, FILE *input);
// run executable file
extern int readvm_exc(FILE *input, int *result);
```

### Compile and run
```
$ make install
$ make build
$ make run
> 3628800
```

### Input .File main.vms
```asm
label begin
    push 10
    call fact
    jmp end
label end
    pop
    hlt

; A <- fact(A)
label fact
    ; B <- A
    load -2
label _fact_for
    ; IF B < 2
    load -1
    push 2
    jl _fact_end
    ; B <- B - 1
    load -1
    push 1
    sub
    store -2 -1
    pop
    ; A <- A * B
    load -3
    load -2
    call mul
    pop
    store -4 -1
    pop
    jmp _fact_for
label _fact_end
    ; return
    pop
    ret

; A <- mul(A, B)
label mul
    ; A'
    load -3
label _mul_for
    ; IF B < 2
    load -3
    push 2
    jl _mul_end
    ; B <- B - 1
    load -3
    push 1
    sub
    store -4 -1
    pop
    ; A <- A + A'
    load -4
    load -2
    add
    store -5 -1
    pop
    jmp _mul_for
label _mul_end
    pop
    ret
```

### Output .File main.vme
```
0000 0000 0a0a 0000 0011 0400 0000 0f01
0c09 ffff fffe 09ff ffff ff00 0000 0002
0500 0000 5909 ffff ffff 0000 0000 0103
08ff ffff feff ffff ff01 09ff ffff fd09
ffff fffe 0a00 0000 5b01 08ff ffff fcff
ffff ff01 0400 0000 1601 0b09 ffff fffd
09ff ffff fd00 0000 0002 0500 0000 9e09
ffff fffd 0000 0000 0103 08ff ffff fcff
ffff ff01 09ff ffff fc09 ffff fffe 0208
ffff fffb ffff ffff 0104 0000 0060 010b
```
