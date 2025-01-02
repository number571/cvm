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
    ; IF 2 > B
    push 2
    push -2
    load
    push _fact_end
    jg
    ; B <- B - 1
    push -1
    load
    dec
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
