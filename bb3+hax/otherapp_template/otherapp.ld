OUTPUT_ARCH(arm)

MEMORY
{
	RAMRO (rx) : ORIGIN = 0x00100000, LENGTH = 0x00002000
	RAMRW (rw!i) : ORIGIN = 0x08000000, LENGTH = 0x0010000
}

SECTIONS
{
	.text : ALIGN(0x1000) {
		build/crt0.o(.init)
		*(.text)
	}

	.rodata ALIGN(0x4) :
	{
		*(.rodata)
		*(.roda)
		*(.rodata.*)
		*all.rodata*(*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);
	}
}