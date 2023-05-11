;
; Check UART
;

	include ../DEVMAP.INC

	include ../BOOTROM/BOOTMEM.INC

	include ../BOOTROM/BOOTROM.INC

	org $100

	sei

	ldx	#hello
	jsr	F_UART_PUTS

	jmp	F_RESET

hello	db	'Hello, World!', 10, 13, 0
