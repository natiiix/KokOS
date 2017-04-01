#include <drivers/io/keyboard.h>
#include <assembly.h>
#include <drivers/io/terminal.h>
#include <drivers/memory.h>

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

// KEB = Key Event Buffer
#define KEB_SIZE 1024
struct keyevent keyEventBuffer[KEB_SIZE];
size_t kebWriteIdx; // Where should the next key event be stored
size_t kebReadIdx; // From where should the next key event be read

const uint16_t PORT_KEYBOARD = 0x60;
bool keyPressed[KEYS_COUNT];

void keybd_init(void)
{
    mem_set(&keyEventBuffer, 0, sizeof(struct keyevent) * KEB_SIZE);

    kebWriteIdx = 0;
    kebReadIdx = 0;

	for (size_t i = 0; i < KEYS_COUNT; i++)
	{
		keyPressed[i] = false;
	}

    term_writeline("Keyboard initialized.", false);
}

void keyboard_handler(void)
{
    uint8_t status = 0;
    uint8_t keycode = 0;

    // Acknowledgment
    status = inb(0x64);
    // Lowest bit of status will be set if buffer is not empty
    if (status & 0x01)
    {
        keycode = inb(0x60);

        struct keyevent ke;

        ke.scancode = keycode & 0x7F;
        ke.state = !(keycode & 0x80);

        keyPressed[ke.scancode] = ke.state;

        ke.shift = (keyPressed[KEY_SHIFT_LEFT] || keyPressed[KEY_SHIFT_RIGHT]);
        ke.ctrl = keyPressed[KEY_CTRL];
        ke.alt = keyPressed[KEY_ALT];

        if (ke.scancode > 57 || !ke.state || ke.ctrl || ke.alt)
        {
            ke.keychar = 0;
        }
        else
        {
            ke.keychar = (ke.shift ? asciiShift[ke.scancode] : asciiDefault[ke.scancode]);
        }

        // Store the key event into the buffer
        keyEventBuffer[kebWriteIdx] = ke;

        // Increment the buffer writing index
        if (++kebWriteIdx >= KEB_SIZE)
        {
            kebWriteIdx = 0;
        }
    }

    // Enable interrupts again
    outb(0x20, 0x20);
}

struct keyevent keybd_read(void)
{
    struct keyevent ke;

    // The buffer is empty
    if (kebReadIdx == kebWriteIdx)
    {
        mem_set(&ke, 0, sizeof(struct keyevent));
        return ke;
    }

    // Read the key event from the buffer
    ke = keyEventBuffer[kebReadIdx];

    // Increment the buffer reading index
    if (++kebReadIdx >= KEB_SIZE)
    {
        kebReadIdx = 0;
    }

    return ke;
}
