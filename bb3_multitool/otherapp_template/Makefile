ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif
export CTRULIB=$(shell pwd)/libctru
ifeq ($(strip $(CTRULIB)),)
$(error "Please set CTRULIB in your environment. export DEVKITARM=<path to>ctrulib/libctru")
endif

ifeq ($(filter $(DEVKITARM)/bin,$(PATH)),)
export PATH:=$(DEVKITARM)/bin:$(PATH)
endif

DEFINES	:=	
LDPATH	:=	ccd00.ld

ifneq ($(strip $(LOADROPBIN)),)
	DEFINES	:=	$(DEFINES) -DLOADROPBIN=1
endif


DEFINES	:=	$(DEFINES) -DOTHERAPP=1
LDPATH	:=	otherapp.ld


CC = arm-none-eabi-gcc
# LINK = arm-none-eabi-gcc
LINK = arm-none-eabi-ld
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
CFLAGS += -Wall -std=c99 -march=armv6 -Os -I"$(CTRULIB)/include" -I$(DEVKITPRO)/libnds/include $(DEFINES)
LDFLAGS += --script=$(LDPATH) -L"$(DEVKITARM)/arm-none-eabi/lib" -L"$(CTRULIB)/lib" -Map=output.map

CFILES = $(wildcard source/*.c)
BINFILES = $(wildcard data/*.bin)
OFILES = $(BINFILES:data/%.bin=build/%.bin.o)
OFILES += $(CFILES:source/%.c=build/%.o)
DFILES = $(CFILES:source/%.c=build/%.d)
SFILES = $(wildcard source/*.s)
OFILES += $(SFILES:source/%.s=build/%.o)
PROJECTNAME = ${shell basename "$(CURDIR)"}
CWD = "$(CURDIR)""

#---------------------------------------------------------------------------------
# canned command sequence for binary data, taken from devkitARM
#---------------------------------------------------------------------------------
define bin2o
	bin2s $< | $(AS) -o $(@)
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > source/`(echo $(<F) | tr . _)`.h
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> source/`(echo $(<F) | tr . _)`.h
	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> source/`(echo $(<F) | tr . _)`.h
endef

.PHONY:=all

all: $(PROJECTNAME).bin

ctrulib:
	#cd "$(CTRULIB)" && make

$(PROJECTNAME).bin: $(PROJECTNAME).elf
	$(OBJCOPY) -O binary $< $@

$(PROJECTNAME).elf: $(OFILES) ctrulib
	$(LINK) $(LDFLAGS) -o $(PROJECTNAME).elf $(filter-out build/crt0.o, $(OFILES)) -g -lctru -lm -lc -lg

clean:
	@rm -f build/*.o build/*.d
	@rm -f $(PROJECTNAME).elf $(PROJECTNAME).bin
	@echo "all cleaned up !"

-include $(DFILES)

build/%.o: source/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(DEFINES) -MM $< > build/$*.d

build/%.o: source/%.s
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(DEFINES) -MM $< > build/$*.d

build/%.bin.o: data/%.bin
	@echo $(notdir $<)
	@$(bin2o)

