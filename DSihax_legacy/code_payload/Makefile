CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy

all:	rop_payload.bin

rop_payload.bin	: rop_payload.elf
	$(OBJCOPY) -O binary $< $@

rop_payload.elf: rop_payload.S
	$(CC)	-nostartfiles -nostdlib -Ttext=0x0058c460 -x assembler-with-cpp $< -o $@

clean:
	@rm -f *.elf *.bin *.dat
