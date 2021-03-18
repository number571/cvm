# VirtualMachine
> Stack-based virtual machine.

### Instructions (num = 16)
1.  [0x00] "push"  
2.  [0x01] "pop"   
3.  [0x02] "add"   
4.  [0x03] "sub"   
5.  [0x04] "mul"   
6.  [0x05] "div"   
7.  [0x06] "jmp"   
8.  [0x07] "jl"    
9.  [0x08] "jg"    
10. [0x09] "je"    
11. [0x0A] "store" 
12. [0x0B] "load"  
13. [0x0C] "call"  
14. [0x0D] "ret"   
15. [0x0E] "hlt"   
16. [0x0F] "alloc" 

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
; Factorial
label begin
    ; A <-
    push 10
    call fact
    jmp end
label end
    ; <- A
    pop
    hlt
; A <- fact(A)
label fact
    ; B <- A
    load -2
label _fact
    ; IF B < 2
    load -1
    push 2
    jl _end
    ; B <- B - 1
    load -1
    push 1
    sub
    store -2 -1
    pop
    ; A <- A * B
    load -3
    load -2
    mul
    store -4 -1
    pop
    jmp _fact
label _end
    ; return
    pop
    ret

```

### Output .File main.vme
```
0000 0000 0a0c 0000 0011 0600 0000 0f01
0e0b ffff fffe 0bff ffff ff00 0000 0002
0700 0000 540b ffff ffff 0000 0000 0103
0aff ffff feff ffff ff01 0bff ffff fd0b
ffff fffe 040a ffff fffc ffff ffff 0106
0000 0016 010d 
```
