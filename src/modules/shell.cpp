#include <modules/shell.hpp>

#include <c/stdlib.h>
#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <kernel.h>

#include <modules/disk.hpp>
#include <drivers/storage/fat.h>

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
		//pause();
	#endif

    clear();

	m_diskToolsEnabled = (partCount > 0);

	if (m_diskToolsEnabled)
	{
		m_activePart = 0;
		m_activeDir = partArray[m_activePart].rootDirCluster;
		m_pathDirs.clear();
		_update_prefix();
	}
	else
	{
		m_prefix.clear();
		m_prefix.push_back('>');
	}

	initModules();

    while (true)
    {
		#ifdef DEBUG
			// Some memory is allocated for constant strings
			debug_memusage();
			//pause();
		#endif

        string strInput = readline();

		sprint(m_prefix);
        sprint(strInput);
		newline();

		process(strInput);
		
		strInput.dispose();
    }
}

void Shell::initModules(void)
{
	m_modDisk.init("disk");
}

void Shell::process(const string& strInput)
{
	// Command syntax explanation:
	// Literal String
	// <Required Argument>
	// [Optional Argument]

	// Extract command string from the input string
	string strCmd;

	size_t strsize = strInput.size();

	for (size_t i = 0; i < strsize && strInput.at(i) != ' '; i++)
	{
		strCmd.push_back(strInput.at(i));
	}

	// Separate arguments from command string
	string strArgs = strInput.substr(strCmd.size() + 1);

	// -- Check internal shell commands --
	// Partition switch
	// Syntax: <Partition Letter>:
	if (strCmd.size() == 2 && strCmd[1] == ':')
	{
		char partLetter = strCmd[0];
		uint8_t partIdx = 0xFF;

		// Translate letter to index
		if (partLetter >= 'A' && partLetter <= 'Z')
		{
			partIdx = partLetter - 'A';
		}
		else if (partLetter >= 'a' && partLetter <= 'z')
		{
			partIdx = partLetter - 'a';
		}

		// If it's a valid partition index switch the active partition
		if (partIdx < partCount)
		{
			m_activePart = partIdx;
			_update_prefix();
		}
		else
		{
			print("Invalid partitions letter!\n");
		}
	}
	// Directory switch
	// Syntax: cd <Directory Path>
	else if (strCmd.compare("cd"))
	{
		// TODO
	}
	// List directory content
	// Syntax: dir [Directory Path]
	else if (strCmd.compare("dir") && vecArgs.size() < 2)
	{
		if (vecArgs.size() == 0)
		{
			listDirectory(m_activePart, m_activeDirectory);
		}
		else
		{
			/*uint8_t partIdx = (uint8_t)strparse(vecArgs[1].c_str(), 10);
			if (partIdx >= partCount)
			{
				print("Invalid partition index!\n");
			}
			else
			{
				uint32_t pathCluster = resolvePath(partIdx, vecArgs[2].c_str());

				if (pathCluster)
				{
					listDirectory(partIdx, pathCluster);
				}
				else
				{
					print("Invalid directory path!\n");
				}
			}*/
		}
	}
	// -- Compare the input string against each module command string --
	// Disk operation module
	// Syntax: disk <Action> <Arguments>
	else if (m_modDisk.compare(strCmd))
	{
		m_modDisk.process(strArgs);
	}
	else
	{
		print("Invalid command: \"");
		sprint(strCmd);
		print("\"\n");
	}

	strCmd.dispose();
	strArgs.dispose();
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

	size_t preLen = m_prefix.size();
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

		sprintat(m_prefix, 0, row);
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

void Shell::_update_prefix(void)
{
	m_prefix.clear();

	if (m_diskToolsEnabled)
	{
		string activePath = string::join(m_pathDirs, '/', true);

		m_prefix.push_back('A' + m_activePart);
		m_prefix.push_back(':');
		m_prefix.push_back(activePath);
		m_prefix.push_back('/');

		activePath.dispose();
	}

	m_prefix.push_back('>');
}
