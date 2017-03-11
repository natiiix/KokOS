#pragma once

#include "stdtypes.hpp"
#include "asm.hpp"

namespace keybd
{
	// Reads a scan code from keyboard and returns it
	uint8_t getScancode(void);
	// Returns char corresponding to specified key scan code
	char scancodeToChar(uint8_t sc, bool shiftPressed = false);

	// Special (non-character) keyboard keys
	enum keys_special : uint8_t
	{
		KEY_ESCAPE = 1,
		KEY_BACKSPACE = 14,
		KEY_TAB = 15,
		KEY_ENTER = 28,
		KEY_CTRL = 29,
		KEY_SHIFT_LEFT = 42,
		KEY_SHIFT_RIGHT = 54,
		KEY_ALT = 56,
		KEY_CAPS_LOCK = 58,
		KEY_SCROLL_LOCK = 70,
	};

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
}
