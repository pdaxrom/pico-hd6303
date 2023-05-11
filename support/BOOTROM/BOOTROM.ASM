;
; BOOTROM for Z/pdaXrom
; (c) sashz <sashz@pdaXrom.org>, 2017-2018
;

	include ../DEVMAP.INC

 .ifdef HD6303Y
 .define HD6303

MEMPAGE_HI	equ	$10

 .endif

	include BOOTMEM.INC

	org	BOOTROM_BASE

	bra	reset
	bra	uart_in_1
	bra	uart_out_1
	bra	uart_puts_1
	bra	uart_outhex_1

reset
	sei

	lds	#$ff

 .ifdef HD6303Y
; Setup page port (P6)
	ldaa	REG_PORT6	; MEMPAGE Port initialization
	anda	#MEMPAGE_FMASK|MEMPAGE_HI
	staa	REG_PORT6
	ldaa	#$FF
	staa	REG_DDRP6
 .endif

; setup stubs

 .ifdef HD6303
 .ifdef HD6303Y
	ldx	#vstub_irq2
	stx	VIRQ2
	ldx	#vstub_cmi
	stx	VCMI
 .endif
	ldx	#vstub_trap
	stx	VTRAP
	ldx	#vstub_sci
	stx	VSCI
	ldx	#vstub_tof
	stx	VTOF
	ldx	#vstub_ocf
	stx	VOCF
	ldx	#vstub_icf
	stx	VICF
 .endif
	ldx	#vstub_irq
	stx	VIRQ
	ldx	#vstub_swi
	stx	VSWI
	ldx	#vstub_nmi
	stx	VNMI

	bsr	uart_init

	ldx	#pylogo
	bsr	uart_puts

loop
	bsr	uart_in
	cmpa	#'L'
	beq	cmd_load
	cmpa	#'S'
	beq	cmd_save
	cmpa	#'G'
	beq	cmd_go
	ldaa	#'E'
	bsr	uart_out
	bra	loop

cmd_load
	bsr	get_block_addr
cmd_lo1	bsr	uart_in
	staa	0,x
	inx
	cpx	END_ADDR
	bne	cmd_lo1
cmd_oka	ldaa	#'O'
	bsr	uart_out
	bra	loop

cmd_save
	bsr	get_block_addr
cmd_sa1	ldaa	0,x
	bsr	uart_out
	inx
	cpx	END_ADDR
	bne	cmd_sa1
	bra	cmd_oka

uart_in_1
	bra	uart_in
uart_out_1
	bra	uart_out
uart_puts_1
	bra	uart_puts
uart_outhex_1
	bra	uart_outhex

cmd_go
	bsr	get_word
	jmp	0,x

get_block_addr proc
	bsr	get_word
	pshx
	bsr	get_word
	stx	END_ADDR
	pulx
	rts
	endp

;; UART IN/OUT

 .if (HD6303 & ~EXTUART)
uart_init proc
	ldaa	#$04		;ENABLE INTERNAL UART, INTERNAL CLOCK, 115200 BAUD
	staa	REG_RMCR
	ldaa	#$0A		;ENABLE RECIEVE AND TRANSMITT DATA
	staa	REG_TRCSR
	ldaa	REG_RDR		;FLUSH BUFFER AND CLEAR ERROR FLAGS
	rts
	endp

uart_in	proc
	ldaa	REG_TRCSR
	anda	#$C0		;FILTER OUT RDRF AND ORFE
	cmpa	#$00
	beq	uart_in		;WAIT FOR CHARACTER
	cmpa	#$40
	beq	uartin1		;CHECK FOR FRAMING ERROR
	ldaa	REG_RDR         ;READ RECIEVED CHARACTER
	rts
uartin1	ldaa	REG_RDR		;ON ERROR, FLUSH BUFFER AND CLEAR ERROR FLAG
	bra	uart_in
	endp

uart_out proc
	pshb
uartou1	ldab	REG_TRCSR	;Get Status Reg 
	aslb			;TDRE->C
	aslb
	aslb
	bcc	uartou1		;READY FOR NEXT CHARACTER
	staa	REG_TDR
	pulb
	rts
	endp
 .else
uart_init proc
; setup prescaler: 12MHz / (Baud * 8)
	ldx	#13
	stx	UART_PRESCALER

; disable UART interrupts (clear UART_TIE and UART_RIE)
	clra
	staa	UART_CONFIG
	rts
	endp

uart_in proc
	ldaa	UART_STATUS
	bita	#UART_RRD
	beq	uart_in
	ldaa	UART_DATA
	rts
	endp

uart_out proc
	pshb
loop	ldab	UART_STATUS
	bitb	#UART_TRD
	beq	loop
	staa	UART_DATA
	pulb
	rts
	endp
 .endif

uart_puts proc
	pshx
	psha
loop	ldaa	0,x
	tsta
	beq	exit
	bsr	uart_out
	inx
	bra	loop
exit	pula
	pulx
	rts
	endp

uart_outhex proc
	psha
	pshb
	tab
	bsr	OUTNIBH
	tba
	bsr	OUTNIBL
	pulb
	pula
	rts
OUTNIBH	lsra
	lsra
	lsra
	lsra
OUTNIBL	anda	#$0F
	oraa	#$30
	cmpa	#$39
	bls	OUTNIBX
	adda	#$7
OUTNIBX bsr	uart_out
	rts
	endp

get_word proc
	bsr	uart_in
	tab
	bsr	uart_in
	psha
	pshb
	pulx
	rts
	endp

pylogo	db	$0a, $0d, 'Z/pdaXrom'
 .ifdef HD6303
 .ifdef HD6303Y
	db	' HD6303Y'
 .else
	db	' HD6303'
 .endif
 .endif
	db	$0a, $0d, 0

;----
inthandlers proc
 .ifdef HD6303
 .ifdef HD6303Y
	global vstub_irq2, vstub_cmi, vstub_trap, vstub_sci, vstub_tof, vstub_ocf, vstub_icf, vstub_irq, vstub_swi, vstub_nmi
vstub_irq2
	ldx	#vectextIRQ2
	bra	ret

vstub_cmi
	ldx	#vectextCMI
	bra	ret

 .else
	global vstub_trap, vstub_sci, vstub_tof, vstub_ocf, vstub_icf, vstub_irq, vstub_swi, vstub_nmi
 .endif
vstub_trap
	ldx	#vectextTRAP
	bra	ret

vstub_sci
	ldx	#vectextSCI
	bra	ret

vstub_tof
	ldx	#vectextTOF
	bra	ret

vstub_ocf
	ldx	#vectextOCF
	bra	ret

vstub_icf
	ldx	#vectextICF
	bra	ret

 .else
	global vstub_irq, vstub_swi, vstub_nmi
 .endif
vstub_irq
	ldx	#vectextIRQ
	bra	ret

vstub_swi
	ldx	#vectextSWI
	bra	ret

vstub_nmi
	ldx	#vectextNMI
ret	bsr	uart_puts
	rti
	endp

 .ifdef HD6303
 .ifdef HD6303Y
vectextIRQ2	db	'IRQ2',0
vectextCMI	db	'CMI',0
 .endif
vectextTRAP	db	'TRAP',0
vectextSCI	db	'SCI',0
vectextTOF	db	'TOF',0
vectextOCF	db	'OCF',0
vectextICF	db	'ICF',0
 .endif
vectextIRQ	db	'IRQ',0
vectextSWI	db	'SWI',0
vectextNMI	db	'NMI',0

;----
intvectors proc
 .ifdef HD6303
 .ifdef HD6303Y
    global vec_irq2, vec_cmi, vec_trap, vec_sci, vec_tof, vec_ocf, vec_icf, vec_irq, vec_swi, vec_nmi
vec_irq2
	ldx	VIRQ2
	jmp	0,x

vec_cmi
	ldx	VCMI
	jmp	0,x

 .else
    global vec_trap, vec_sci, vec_tof, vec_ocf, vec_icf, vec_irq, vec_swi, vec_nmi
 .endif
vec_trap
	ldx	VTRAP
	jmp	0,x

vec_sci
	ldx	VSCI
	jmp	0,x

vec_tof
	ldx	VTOF
	jmp	0,x

vec_ocf
	ldx	VOCF
	jmp	0,x

vec_icf
	ldx	VICF
	jmp	0,x
 .else
	global vec_irq, vec_swi, vec_nmi
 .endif

vec_irq
	ldx	VIRQ
	jmp	0,x

vec_swi
	ldx	VSWI
	jmp	0,x

vec_nmi
	ldx	VNMI
	jmp	0,x
	endp

 .ifdef HD6303
 .ifdef HD6303Y
	ds	$ffea-*, $ff
	dw	vec_irq2
	dw	vec_cmi
 .else
	ds	$ffee-*, $ff
 .endif
	dw	vec_trap
	dw	vec_sci
	dw	vec_tof
	dw	vec_ocf
	dw	vec_icf
 .else
	ds	$fff8-*, $ff
 .endif
	dw	vec_irq
	dw	vec_swi
	dw	vec_nmi
	dw	reset
