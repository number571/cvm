# VirtualMachine
> Stack-based virtual machine.

### Instructions (num = 12)
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
    push 0
    push 0
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
    push 0
    push 0
    je _fact_for
label _fact_end
    ; return
    pop
    ret

; A <- mul(A, B)
label mul
    ; A' <- A
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
    push 0
    push 0
    je _mul_for
label _mul_end
    pop
    ret
```

### Output .File main.vme
```
0000 0000 0a09 0000 001b 0000 0000 0000
0000 0000 0600 0000 1901 0b08 ffff fffe
08ff ffff ff00 0000 0002 0400 0000 6d08
ffff ffff 0000 0000 0103 07ff ffff feff
ffff ff01 08ff ffff fd08 ffff fffe 0900
0000 6f01 07ff ffff fcff ffff ff01 0000
0000 0000 0000 0000 0600 0000 2001 0a08
ffff fffd 08ff ffff fd00 0000 0002 0400
0000 bc08 ffff fffd 0000 0000 0103 07ff
ffff fcff ffff ff01 08ff ffff fc08 ffff
fffe 0207 ffff fffb ffff ffff 0100 0000
0000 0000 0000 0006 0000 0074 010a 
```
