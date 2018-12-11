OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
	. = 0x100200;

	.text : {
        *(.loader_start)
		*(.text .text.*)
	}

	/DISCARD/ : {
		*(.iplt)
		*(.eh_frame .note.GNU-stack)
        *(.note.gnu.build-id)
        *(.comment)
	}
}
