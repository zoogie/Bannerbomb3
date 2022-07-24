#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/base_tools

HAXNAME := bb3

.PHONY: all clean

all:	$(HAXNAME).bin

clean:
	@rm -f $(HAXNAME).elf $(HAXNAME).bin
	@echo clean ...

$(HAXNAME).bin: $(HAXNAME).elf
	$(OBJCOPY) -O binary $(HAXNAME).elf $(HAXNAME).bin
	@echo built ... $(notdir $@)

$(HAXNAME).elf:	$(HAXNAME).s
	$(CC) -x assembler-with-cpp -nostartfiles -nostdlib -Ttext=0x00682000 $< -o $(HAXNAME).elf

