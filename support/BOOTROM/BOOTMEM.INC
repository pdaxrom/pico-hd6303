BOOTROM_BASE	equ	$FE00

		section	$40
VIRQ2		ds	2
VCMI		ds	2
VTRAP		ds	2
VSCI		ds	2
VTOF		ds	2
VOCF		ds	2
VICF		ds	2
VIRQ		ds	2
VSWI		ds	2
VNMI		ds	2
END_ADDR	ds	2
		ends
