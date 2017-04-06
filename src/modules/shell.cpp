#include <modules/shell.hpp>

#include <c/stdlib.h>
#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <kernel.h>

#include <modules/disk.hpp>

extern "C"
void shell_init(void)
{
	Shell shellInstance;
	shellInstance.init();
}

void Shell::init(void)
{
	#ifdef DEBUG
		// Used to make sure there is no memory leaking during kernel initialization
		debug_memusage();
		// Give the user a chance to see kernel initialization messages
		pause();
	#endif

    clear();

	m_strPrefix.clear();
    m_strPrefix.push_back(">");

	initModules();

    while (true)
    {
        string strInput = readline();

		sprint(m_strPrefix);
        sprint(strInput);
		newline();

		process(strInput);
		
		strInput.dispose();

		#ifdef DEBUG
			// Some memory is allocated for constant strings
			debug_memusage();
			pause();
		#endif
    }
}

void Shell::initModules(void)
{
	Disk modDisk;
	modDisk.init("disk");
	m_modules.push_back(modDisk);
}

void Shell::process(const string& strInput)
{
	// Extract command string from the input string
	string strCmd;

	size_t strsize = strInput.size();

	for (size_t i = 0; i < strsize && strInput.at(i) != ' '; i++)
	{
		strCmd.push_back(strInput.at(i));
	}

	// Compare the input string against each module command string
	size_t modsize = m_modules.size();

	for (size_t i = 0; i < modsize; i++)
	{
		if (m_modules[i].compare(strCmd))
		{
			strCmd.dispose();
			return;
		}
	}

	print("Invalid command: \"");
	sprint(strCmd);
	print("\"\n");

	strCmd.dispose();
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

	size_t preLen = m_strPrefix.size();
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

		sprintat(m_strPrefix, 0, row);
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
