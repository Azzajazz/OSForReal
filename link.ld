/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

MEMORY
{
    BOOTPHYS (rwx): ORIGIN = 1M, LENGTH = 1M
    KERNELVIRT (rwx): ORIGIN = 0xC0000000, LENGTH = 8M
}

VIRT_PHYS_OFFSET = 0xC0000000 - 0x200000;

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* It used to be universally recommended to use 1M as a start offset,
	   as it was effectively guaranteed to be available under BIOS systems.
	   However, UEFI has made things more complicated, and experimental data
	   strongly suggests that 2M is a safer place to load. In 2016, a new
	   feature was introduced to the multiboot2 spec to inform bootloaders
	   that a kernel can be loaded anywhere within a range of addresses and
	   will be able to relocate itself to run from such a loader-selected
	   address, in order to give the loader freedom in selecting a span of
	   memory which is verified to be available by the firmware, in order to
	   work around this issue. This does not use that feature, so 2M was
	   chosen as a safer option than the traditional 1M. */
    /* I don't care though, and I know that my current QEMU setup will allow this. */
    /* @FIXME: Find a more portable way of doing this */
	. = 1M;

    __boot_start = .;
    .boot :
    {
        KEEP(*(.multiboot))
        KEEP(*(.stack))
        *(.boot_text)
        *(.boot_data)
        *(.boot_bss)
    } AT> BOOTPHYS
    __boot_end = ALIGN(4K);

    __kernel_phys_start = 2M;
	.text : AT(ADDR(.text) - VIRT_PHYS_OFFSET)
	{
        __kernel_start = .;
		*(.text)
	} > KERNELVIRT

	.rodata : AT(ADDR(.rodata) - VIRT_PHYS_OFFSET)
	{
		*(.rodata)
        *(.rodata.*)
	} > KERNELVIRT

	.data : AT(ADDR(.data) - VIRT_PHYS_OFFSET)
	{
		*(.data)
	} > KERNELVIRT

	/* Read-write data (uninitialized) and stack */
	.bss : AT(ADDR(.bss) - VIRT_PHYS_OFFSET)
	{
		*(COMMON)
		*(.bss)
	} > KERNELVIRT

    . = ALIGN(4K);
    __kernel_end = .;
    __kernel_phys_end = __kernel_phys_start + (__kernel_end - __kernel_start);

    /DISCARD/ :
    {
        *(.comment)
    }
}
