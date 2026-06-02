ENTRY(_entry)

SECTIONS
{
    .text :
    {
        *(.text*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(.bss*)
    }

    /DISCARD/ :
    {
        *
    }
}
