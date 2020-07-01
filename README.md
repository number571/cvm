# VirtualMachine
> Stack-based virtual machine.

### Instructions (num = 16)
1.  [0x00] push  
2.  [0x01] pop   
3.  [0x02] add   
4.  [0x03] sub   
5.  [0x04] mul   
6.  [0x05] div   
7.  [0x06] jmp   
8.  [0x07] jl    
9.  [0x08] jg    
10. [0x09] je    
11. [0x0A] jne   
12. [0x0B] store 
13. [0x0C] load  
14. [0x0D] call  
15. [0x0E] ret   
16. [0x0F] hlt   

### Pseudo instructions (num = 2)
1. label
2. ;

### Interface functions
```c
// translate source file (input) into executable (output)
extern int8_t readvm_src(FILE *output, FILE *input);
// run executable file
extern int32_t readvm_exc(FILE *input);
```

### Compile and run
```
$ make -C extclib/
$ make build
$ make run
```

### Example (Factorial [Source])
```asm
; Factorial
label begin
    ; A
    push 10
    call fact
    jmp end

label end
    ; A
    pop
    hlt

; A <- fact(A)
label fact
    ; B <- A
    load $-2
    ; if B < 2
    push 2
    load $-2
    jl _clfact
label _fact
    ; B <- B - 1
    push 1
    load $-2
    sub
    store $-2 $-1
    pop
    ; A <- A * B
    load $-3
    load $-2
    mul
    store $-4 $-1
    pop
    ; if B > 1
    push 1
    load $-2
    jg _fact
label _clfact
    ; return
    pop
    ret
```

### Example (Factorial [Executable])
```
0000 0000 0a0d 0000 0011 0600 0000 0f01
0f0c ffff fffe 0000 0000 020c ffff fffe
0700 0000 5e00 0000 0001 0cff ffff fe03
0bff ffff feff ffff ff01 0cff ffff fd0c
ffff fffe 040b ffff fffc ffff ffff 0100
0000 0001 0cff ffff fe08 0000 0025 010e
```
