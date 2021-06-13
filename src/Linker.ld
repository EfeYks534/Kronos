ENTRY(KernelBoot)

SECTIONS
{
	. = 0xFFFFFFFF80200000;

	.stivale2hdr : {
	    KEEP(*(.stivale2hdr))
	}

	.text : {
	    *(.text*)
	}

	.ctors : {
		kdinit_begin = .;

		*(.ctors.65535)

		kdinit_end = .;

		keinit_begin = .;

		*(.ctors.65534)

		keinit_end = .;

		klinit_begin = .;

		*(.ctors.65533)

		klinit_end = .;

	}

	.rodata : {
	    *(.rodata*)
	}

	.data : {
	    *(.data*)
	}

	.bss : {
	    *(COMMON)
	    *(.bss*)
	}
}
