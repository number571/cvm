# VirtualMachine
> Stack-based virtual machine.

### Operations (num = 18)
1.  push  [0x00]
2.  pop   [0x01]
3.  add   [0x02]
4.  sub   [0x03]
5.  mul   [0x04]
6.  div   [0x05]
7.  jmp   [0x06]
8.  jl    [0x07]
9.  jg    [0x08]
10. je    [0x09]
11. jne   [0x0A]
12. store [0x0B]
13. load  [0x0C]
14. call  [0x0D]
15. ret   [0x0E]
16. hlt   [0x0F]
17. label [none]
18. ;     [none]


### Interface functions
```c
// translate source file (input) into executable (output)
extern int32_t readvm_src(FILE *output, FILE *input);
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
