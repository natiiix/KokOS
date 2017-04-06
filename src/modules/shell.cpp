#include <modules/shell.hpp>

#include <c/stdlib.h>
#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <kernel.h>

extern "C"
void shell_init(void)
{
	Shell shellInstance;
	shellInstance.init();
}

void Shell::init(void)
{
	#ifdef DEBUG
		debug_memusage();
		// Give the user a chance to see kernel initialization messages
		pause();
	#endif

    clear();

	strPrefix.clear();
    strPrefix.push_back(">");

    while (true)
    {
        string strInput = readline();

		sprint(strPrefix);
        sprint(strInput);
		newline();
		
		strInput.dispose();

        debug_memusage(); // 0x100 is used by strPrefix
		pause();
    }
}

char* Shell::_generate_spaces(const size_t count)
{
	char* strspaces = (char*)malloc(count + 1);
	strspaces[count] = '\0';
	
	for (size_t i = 0; i < count; i++)
	{
		strspaces[i] = ' ';
	}

	return strspaces;
}

string Shell::readline(void)
{
    if (getcol() > 0)
	{
		newline();
	}

	size_t row = getrow();

	size_t preLen = strPrefix.size();
	size_t inSpace = VGA_WIDTH - preLen - 1;

	string strInput;

	while (true)
	{
		struct keyevent ke = readKeyEvent();
			
		if (ke.state)
		{			
			if (ke.keychar > 0)
			{
				strInput += ke.keychar;
			}
			else if (ke.scancode == KEY_ENTER)
			{
				char* strspaces = _generate_spaces(VGA_WIDTH);
				printat(strspaces, 0, row);
				delete strspaces;

				return strInput;
			}
			else if (ke.scancode == KEY_BACKSPACE)
			{
				if (strInput.size() > 0)
				{
					strInput.pop_back();
				}
			}
			else if (ke.scancode == KEY_ESCAPE)
			{
				strInput.clear();
			}
		}

		size_t inRenderLen = (inSpace >= strInput.size() ? strInput.size() : inSpace);
		size_t inStartIdx = strInput.size() - inRenderLen;

		sprintat(strPrefix, 0, row);
		string strInputRender = strInput.substr(inStartIdx, inRenderLen);
		sprintat(strInputRender, preLen, row);
		strInputRender.dispose();

		size_t emptySpace = inSpace - inRenderLen;
		size_t rowend = preLen + inRenderLen;

		if (emptySpace > 0)
		{
			char* strspaces = _generate_spaces(emptySpace);
			printat(strspaces, rowend, row);
			delete strspaces;
		}

		setcursor(rowend, row);
	}
}
