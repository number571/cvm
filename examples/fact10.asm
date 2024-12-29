labl begin
    push 10
    push fact
    call
    push end
    jmp
labl end
    hlt

; A <- fact(A)
labl fact
    ; B <- A
    push -2
    load
labl _fact_for
    ; IF B < 2
    push _fact_end
    push -2
    load
    push 2
    jl
    ; B <- B - 1
    push -1
    load
    push 1
    sub
    push -1
    push -2
    stor
    pop
    ; A <- A * B
    push -3
    load
    push -2
    load
    mul
    push -1
    push -4
    stor
    pop
    push _fact_for
    jmp
labl _fact_end
    ; return
    pop
    jmp
