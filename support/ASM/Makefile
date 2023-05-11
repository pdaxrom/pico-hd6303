UNIASM = ../../hd6303-toolchain/unias

all: $(UNIASM)		\
    UART.CMD		\


#    RAMCHK.CMD		\
#    RAMTEST.CMD		\
#    MICROBASIC.CMD	\
#    MON.BIN

$(UNIASM):
	make -C $(shell dirname $(UNIASM)) $(shell basename $(UNIASM))

.SUFFIXES: .ASM .CMD

.ASM.CMD: ../DEVMAP.INC ../BOOTROM.INC
	$(UNIASM) $< $(basename $@) $@

clean:
	rm -f *.LST *.CMD ymstreamer *.BIN
