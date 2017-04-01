#include <drivers/io/keyboard.h>
#include <assembly.h>
#include <io/terminal.h>

const char asciiDefault[] =
{
    0,
    0, 	// ESC
    49,	// 1
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57, // 9
    48,	// 0
    45,	// -
    61,	// =
    0,
    0,
    113,// q
    119,
    101,
    114,
    116,
    121,
    117,
    105,
    111,
    112,// p
    91,	// [
    93,	// ]
    0,
    0,
    97,	// a
    115,
    100,
    102,
    103,
    104,
    106,
    107,
    108,// l
    59,	// ;
    39,	// '
    96,	// `
    0,
    92,	// BACKSLASH
    122,// z
    120,
    99,
    118,
    98,
    110,
    109,// m
    44,	// ,
    46,	// .
    47,	// /
    0,
    42,	// *
    0,
    32,	// SPACE
};

const char asciiShift[] =
{
    0,
    0, 	// ESC
    33,	// !
    64, // @
    35, // #
    36, // $
    37, // %
    94, // ^
    38, // &
    42, // *
    40, // (
    41,	// )
    95,	// _
    43,	// +
    0,
    0,
    81, // Q
    87,
    69,
    82,
    84,
    89,
    85,
    73,
    79,
    80, // P
    123,// {
    125,// }
    0,
    0,
    65,	// A
    83,
    68,
    70,
    71,
    72,
    74,
    75,
    76, // L
    58,	// :
    34,	// "
    126,// ~
    0,
    124,// |
    90, // Z
    88,
    67,
    86,
    66,
    78,
    77, // M
    60,	// <
    62,	// >
    63,	// ?
    0,
    0,
    0,
    32,	// SPACE
};

#define KEY_BUFFER_SIZE 1024
uint8_t keybuffer[KEY_BUFFER_SIZE];
size_t keybufferWriteIdx; // Where should the next key event be stored
size_t keybufferReadIdx; // From where should the next key event be read

const uint16_t PORT_KEYBOARD = 0x60;
bool keyPressed[KEYS_COUNT];

void keybd_init(void)
{
    for (size_t i = 0; i < KEY_BUFFER_SIZE; i++)
    {
        keybuffer[i] = 0;
    }

    keybufferWriteIdx = 0;
    keybufferReadIdx = 0;

	for (size_t i = 0; i < KEYS_COUNT; i++)
	{
		keyPressed[i] = false;
	}

    term_writeline("Keyboard initialized.", false);
}

// ---- OBSOLETE CODE ----
// Reads a pressed key scancode from keyboard using polling
/*uint8_t keybd_readkey(void)
{
	uint8_t c = 0;
	
	do
	{
		if (inb(PORT_KEYBOARD) != c)
		{
			c = inb(PORT_KEYBOARD);

			if (c > 0)
			{
				return c;
			}
		}
	}
	while (true);
}*/

char scancodeToChar(const uint8_t scancode, const bool shiftPressed)
{
	// Scan code 57 is the last character key
	// Key with scan code above 57 is guaranteed to be non-character
	if (scancode > 57)
	{
		return 0;
	}

    return (shiftPressed ? asciiShift[scancode] : asciiDefault[scancode]);
}

void keyboard_handler(void)
{
    uint8_t status;
    uint8_t keycode;

    // Acknowledgment
    status = inb(0x64);
    // Lowest bit of status will be set if buffer is not empty
    if (status & 0x01)
    {
        keycode = inb(0x60);

        if (keycode)
        {
            // Store the key event into the buffer
            keybuffer[keybufferWriteIdx] = keycode;

            // Increment the buffer writing index
            if (++keybufferWriteIdx >= KEY_BUFFER_SIZE)
            {
                keybufferWriteIdx = 0;
            }
        }
    }

    // Enable interrupts again
    outb(0x20, 0x20);
}

// Reads a pressed key scancode from key buffer (no polling)
// If there are no key entets in the buffer return 0
uint8_t keybd_readkey(void)
{
    if (keybufferReadIdx < keybufferWriteIdx)
    {
        // Read the key event from the buffer
        uint8_t keycode = keybuffer[keybufferReadIdx];

        // Delete the key event buffer entry to make sure it won't accidentally get read again
        keybuffer[keybufferReadIdx] = 0;

        // Increment the buffer reading index
        if (++keybufferReadIdx >= KEY_BUFFER_SIZE)
        {
            keybufferReadIdx = 0;
        }

        return keycode;
    }
    else
    {
        return 0;
    }
}
