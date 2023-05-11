all: BOOTROM.BIN bootloader

UNIAS = ../../hd6303-toolchain/unias

SED = sed

ASOPTS = -D HD6303 -D EXTUART

$(UNIAS):
	make -C $(shell dirname $(UNIAS))

BOOTROM.BIN: $(UNIAS)
	$(UNIAS) $(ASOPTS) -l BOOTROM.LST -o BOOTROM.BIN BOOTROM.ASM

bootrom.h: BOOTROM.BIN
	xxd -i BOOTROM.BIN > bootrom.h
	$(SED) -i 's/unsigned char BOOTROM_BIN/const unsigned char __in_flash\("bootrom_file"\) bootrom_file/g' bootrom.h
	$(SED) -i 's/BOOTROM_BIN/bootrom_file/g' bootrom.h

clean:
	rm -f *.BIN *.LST *.CMD bootloader *.o bootrom.h

flash: BOOTROM.BIN
	minipro -p AT29C010A -w uartboot.bin

bootloader: bootloader.c
	$(CC) $^ -o $@ -Wall -O2 -g
