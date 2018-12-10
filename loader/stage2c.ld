OUTPUT_FORMAT("elf32-x86-64")

SECTIONS
{
	. = 0xffff800000100800;

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
