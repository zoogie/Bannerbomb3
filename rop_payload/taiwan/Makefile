REGION := USA
ifeq ($(strip $(REGION)),)
$(error "Set the REGION Makefile parameter.")
endif

HAXID	:=	0
HAXNAME := rop_payload



REGCODE	:=	00
ifeq ($(REGION),USA)
	REGCODE	:=	45
endif
ifeq ($(REGION),EUR)
	REGCODE	:=	56
endif
ifeq ($(REGCODE),00)
	$(error "Unsupported region.")
endif

all:	rop_payload.bin

clean:
	rm -f $(HAXNAME).elf rop_payload.bin

rop_payload.bin: $(HAXNAME).elf
	arm-none-eabi-objcopy -O binary $(HAXNAME).elf rop_payload.bin

$(HAXNAME).elf:	$(HAXNAME).s
	arm-none-eabi-gcc -x assembler-with-cpp -nostartfiles -nostdlib -Ttext=0x0ffffe28 $< -o $(HAXNAME).elf

