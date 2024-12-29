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
    push caesar_exit
    push -2
    load
    push -5
    load
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
    jmp

