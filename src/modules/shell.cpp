#include <c/stdlib.h>
#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <kernel.h>

string strPrefix;

extern "C"
void shell_init(void)
{
    clear();

    strPrefix = ">";

    while (true)
    {
        char* cstrinput = shell_readline();
        string strinput = cstrinput;
        delete cstrinput;

        vector<string> words = strinput.split(' ', true);
        if (words.size() > 0)
        {
            print(words[0].c_str());
            print("\n");
        }
        
        strinput.dispose();
        words.dispose();

        debug_memusage();
    }
}

void _print_input_temp(const string& strInput)
{
    size_t preLen = strPrefix.size();
    size_t inSpace = VGA_WIDTH - preLen - 1;
    size_t inLen = strInput.size();
    size_t inRenderLen = (inSpace >= inLen ? inLen : inSpace);
    size_t inStartIdx = inLen - inRenderLen;

    print(strPrefix);
    print(strInput.substr(inStartIdx, inRenderLen));
}

void _print_input_clean(void)
{

}

string shell_readline(void)
{
    if (getcol() > 0)
	{
		newline();
	}

    size_t row = getrow();
	string strInput;

	while (true)
	{
		uint8_t scancode = readkey();
		uint8_t keycode = scancode & 0b01111111;
		bool keystate = (scancode == keycode);

		if (keyPressed[keycode] != keystate)
		{
			keyPressed[keycode] = keystate;
			
			if (keystate)
			{
				char inchar = keytochar(keycode, keyPressed[KEY_SHIFT_LEFT] || keyPressed[KEY_SHIFT_RIGHT]);
				
				if (inchar > 0)
				{
					strInput += inchar;
				}
				else if (keycode == KEY_ENTER)
				{
					_print_input_clean();
					return strInput;
				}
				else if (keycode == KEY_BACKSPACE)
				{
					// Make sure the cursor doesn't get past the beginning of the input row
					if (bufferptr > 0)
						strInput[--bufferptr] = '\0';
				}    
				else if (keycode == KEY_ESCAPE)
				{
					strInput[bufferptr = 0] = '\0';
				}
			}
		}

		_print_input_temp(strInput);
	}
}
