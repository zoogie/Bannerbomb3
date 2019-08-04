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

OUTNAME = "otherapp"
PAYLOAD_SRCPATH	:=	cn_secondary_payload/cn_secondary_payload.bin

SCRIPTS = "scripts"

all: $(OUTNAME).bin

$(OUTNAME).bin: $(PAYLOAD_SRCPATH)
	@cp $(PAYLOAD_SRCPATH) $(OUTNAME).bin

cn_secondary_payload/cn_secondary_payload.bin:
	@mkdir -p cn_secondary_payload/data
	@cd cn_secondary_payload && make
clean:
	@rm -rf build/*
	@cd cn_secondary_payload && make clean
	@echo "all cleaned up !"