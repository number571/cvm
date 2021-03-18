# VirtualMachine
> Stack-based virtual machine.

### Instructions (num = 13)
1.  [0x00] "push"  
2.  [0x01] "pop"   
3.  [0x02] "add"   
4.  [0x03] "sub"   
5.  [0x04] "jl"    
6.  [0x05] "jg"    
7.  [0x06] "je"    
8.  [0x07] "store" 
9.  [0x08] "load"  
10. [0x09] "call"  
11. [0x0A] "ret"   
12. [0x0B] "hlt"   
13. [0x0C] "alloc" 

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
    alloc 2
    je end
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
    alloc 2
    je _fact_for
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
    alloc 2
    je _mul_for
label _mul_end
    pop
    ret
```

### Output .File main.vme
```
0000 0000 0a09 0000 0016 0c00 0000 0206
0000 0014 010b 08ff ffff fe08 ffff ffff
0000 0000 0204 0000 0063 08ff ffff ff00
0000 0001 0307 ffff fffe ffff ffff 0108
ffff fffd 08ff ffff fe09 0000 0065 0107
ffff fffc ffff ffff 010c 0000 0002 0600
0000 1b01 0a08 ffff fffd 08ff ffff fd00
0000 0002 0400 0000 ad08 ffff fffd 0000
0000 0103 07ff ffff fcff ffff ff01 08ff
ffff fc08 ffff fffe 0207 ffff fffb ffff
ffff 010c 0000 0002 0600 0000 6a01 0a
```
