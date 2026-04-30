/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

MEMORY
{
    BOOTPHYS (rwx): ORIGIN = 1M, LENGTH = 1M
    KERNELPHYS (rwx): ORIGIN = 2M, LENGTH = 8M
    KERNELVIRT (rwx): ORIGIN = 0xC0000000, LENGTH = 8M
}

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

    __kernel_phys_start = ORIGIN(KERNELPHYS);
	.text :
	{
        __kernel_start = .;
		*(.text)
	} > KERNELVIRT AT> KERNELPHYS 

	/* Read-only data. */
    /* @Hack: The alignment constraint here is taken from the non-debug build of the 
     * kernel. It's necessary to keep the VMA and LMA of the kernel in sync. */
    /* @TODO: Probably we could load these sections at a constant offset between VMA and LMA */
	.rodata : ALIGN(32)
	{
		*(.rodata)
        *(.rodata.*)
	} > KERNELVIRT AT> KERNELPHYS 

	/* Read-write data (initialized) */
	.data :
	{
		*(.data)
	} > KERNELVIRT AT> KERNELPHYS 

	/* Read-write data (uninitialized) and stack */
	.bss :
	{
		*(COMMON)
		*(.bss)
	} > KERNELVIRT AT> KERNELPHYS 

    . = ALIGN(4K);
    __kernel_end = .;
    __kernel_phys_end = __kernel_phys_start + (__kernel_end - __kernel_start);

    /DISCARD/ :
    {
        *(.comment)
    }
}
