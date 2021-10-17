labl _start
	push begin 
	jmp

; main
labl begin
	; mul5(x) = x * 5
	; where x = 10
	push 10
	push mul5
	call
	push end
	jmp	

; exit 
labl end
	hlt

; x = arg[1]
labl mul5
	; y = x * 5
	push -2
	load 
	push 5
	mul

	; x = y
	push -1
	push -3
	stor 

	; return
	pop
	jmp
