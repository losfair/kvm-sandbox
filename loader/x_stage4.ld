/* Simple linker script for the JOS kernel.
   See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x08048000;

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
