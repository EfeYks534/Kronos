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
		keinit_begin = .;

		*(.ctors.65535)

		keinit_end = .;

		klinit_begin = .;

		*(.ctors.65534)

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
