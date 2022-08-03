#include "input.h"
#include <string.h>

/* `prompt_for_line()`
 *
 * This function accepts the memory address of a character
 * array and gets input from the user, storing the characters
 * in the given buffer. It will handle backspace and will
 * limit when reaching MAX_LINE - 1.
 *****************************/


void prompt_for_line(char* buf) {
	int j = 0;
	
	cio_print((char *) "> ");
	char c = cio_getc();

	//check if return is entered
	while (c != '\r') {
		// check if backspace is entered
		if (c == 0x08) {
			if (j != 0) {
				buf[--j] = 0;
				cio_printc(0x08);
				cio_printc(' ');
				cio_printc(0x08);
			}
		}
		else {
			if (c == 'c'||c == 'y'||c == 'm'||c == 'k'||c == 'r'||c == 'g'||c == 'b'||c == 'w'){
			if (j < MAX_LINE - 1){
				if (c < 123 && c > 96){
					c-=0x20;
				}
					buf[j++] = c;
					cio_printc(c);
			}
			}
		}

		c = cio_getc();
	}

	buf[j] = '\0';	//terminator
}

