/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

MEMORY
{
    /* We don't really know a length here, but it's surely more than 1M*/
    BIGBLOCK (rwx): ORIGIN = 1M, LENGTH = 1M
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

	/* First put the multiboot header, as it is required to be put very early
	   in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
    .boot :
    {
        __boot_start = .;
        *(.multiboot)
		*boot.o (*)
        *bootstrap.o (*)
        __boot_end = .;
    } AT> BIGBLOCK
    __kernel_phys_start = ALIGN(4K);

    . = 0xC0000000;
	.text BLOCK(4K) : ALIGN(4K)
	{
        __kernel_start = .;
		EXCLUDE_FILE(*boot.o *bootstrap.o) *(.text)
	} AT> BIGBLOCK 

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		EXCLUDE_FILE(*boot.o *bootstrap.o) *(.rodata)
        EXCLUDE_FILE(*boot.o *bootstrap.o) *(.rodata.*)
	} AT> BIGBLOCK

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		EXCLUDE_FILE(*boot.o *bootstrap.o) *(.data)
	} AT> BIGBLOCK

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		EXCLUDE_FILE(*boot.o *bootstrap.o) *(COMMON)
		EXCLUDE_FILE(*boot.o *bootstrap.o) *(.bss)
	} AT> BIGBLOCK

    .eh_frame BLOCK(4K) : ALIGN(4K)
    {
        *(.eh_frame)
    } AT> BIGBLOCK

    . = ALIGN(4K);
    __kernel_end = .;
    __kernel_phys_end = __kernel_phys_start + (__kernel_end - __kernel_start);

    /DISCARD/ :
    {
        *(.comment)
    }

    __loaded_size = (__boot_end - __boot_start) + (__kernel_end - __kernel_start);
}
