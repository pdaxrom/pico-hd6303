all: BOOTROM.BIN bootloader

UNIAS = ../../hd6303-toolchain/unias

SED = sed

ASOPTS = -D HD6303 -D EXTUART

$(UNIAS):
	make -C $(shell dirname $(UNIAS))

BOOTROM.BIN: $(UNIAS)
	$(UNIAS) $(ASOPTS) -l BOOTROM.LST -o BOOTROM.BIN BOOTROM.ASM

$(H_OUT_DIR)bootrom.h: BOOTROM.BIN
	xxd -i BOOTROM.BIN > $(H_OUT_DIR)bootrom.h
	$(SED) -ie 's/unsigned char BOOTROM_BIN/const unsigned char __in_flash\("bootrom_file"\) bootrom_file/g' $(H_OUT_DIR)bootrom.h
	$(SED) -ie 's/BOOTROM_BIN/bootrom_file/g' $(H_OUT_DIR)bootrom.h

clean:
	rm -f *.BIN *.LST *.CMD bootloader *.o bootrom.h*
	rm -rf bootloader.dSYM
	make -C $(shell dirname $(UNIAS)) clean

bootloader: bootloader.c
	$(CC) $^ -o $@ -Wall -O2 -g
