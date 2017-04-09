#include <modules/shell.hpp>

#include <c/stdlib.h>
#include <c/stdio.h>
#include <c/string.h>

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
		m_pathStructure.clear();
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
			// Keep in mind that some memory is always allocated by the shell instance itself
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
		// Read the command word, automatically convert it to lowercase
		strCmd.push_back(ctolower(strInput.at(i)));
	}

	// Separate arguments from command string
	string strArgs = strInput.substr(strCmd.size() + 1);
	vector<string> vecArgs = strArgs.split(' ', true);

	// -- Check internal shell commands --
	// Displays all available commands and their proper syntax
	if (strCmd == "help" && vecArgs.size() == 0)
	{
		// TODO
	}
	// Partition switch
	// Syntax: <Partition Letter>:
	else if (strCmd.size() == 2 && strCmd[1] == ':')
	{
		if (strCmd[0] >= 'a' && strCmd[0] <= 'z' && // letters represent partitions index
			(strCmd[0] - 'a') < partCount) // check partition index validity
		{
			m_activePart = strCmd[0] - 'a'; // change the active partition
			m_pathStructure.clear(); // clear the path because it doesn't exist on this partition
			_update_prefix();
		}
		else
		{
			print("Invalid partition letter!\n");
		}
	}
	// Directory switch
	// Syntax: cd <Directory Path>
	else if (strCmd.compare("cd") && vecArgs.size() == 1)
	{
		uint32_t newDir = resolvePath(m_activePart, m_activeDir, vecArgs[0].c_str());

		if (newDir)
		{
			// If the directory path is absolute delete the old path
			if (vecArgs[0][0] == '/')
			{
				m_pathStructure.clear();
			}

			m_activeDir = newDir;

			vector<string> pathElements = vecArgs[0].split('/', true);
			
			for (size_t i = 0; i < pathElements.size(); i++)
			{
				// Ignore self-pointing path elements
				if (pathElements[i] == ".")
				{
					continue;
				}
				// When going one directory up remove the last directory from the vector
				else if (pathElements[i] == "..")
				{
					m_pathStructure.pop_back();
				}
				// When going one directory down append the directory to the vector
				else
				{
					// The directory name string must not be disposed
					// Therefore we need to copy it into a separate string outside the vector
					// This is done automatically when converting the name to uppercase
					m_pathStructure.push_back(pathElements[i].toupper());
				}
			}

			pathElements.dispose();

			_update_prefix();
		}
		else
		{
			print("Invalid directory path!\n");
		}
	}
	// List directory content
	// Syntax: dir [Directory Path]
	else if (strCmd.compare("dir") && vecArgs.size() < 2)
	{
		if (vecArgs.size() == 0)
		{
			listDirectory(m_activePart, m_activeDir);
		}
		else
		{			
			uint32_t pathCluster = resolvePath(m_activePart, m_activeDir, vecArgs[0].c_str());

			if (pathCluster)
			{
				listDirectory(m_activePart, pathCluster);
			}
			else
			{
				print("Invalid directory path!\n");
			}
		}
	}
	// -- Compare the input string against each module command string --
	// Disk operation module
	// Syntax: disk <Action> <Arguments>
	else if (m_modDisk.compare(strCmd))
	{
		m_modDisk.process(vecArgs);
	}
	else
	{
		print("Invalid command: \"");
		sprint(strCmd);
		print("\"\n");
	}

	strCmd.dispose();
	strArgs.dispose();
	vecArgs.dispose();
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
		string activePath = string::join(m_pathStructure, '/', true);

		m_prefix.push_back('A' + m_activePart);
		m_prefix.push_back(':');
		m_prefix.push_back(activePath);
		m_prefix.push_back('/');

		activePath.dispose();
	}

	m_prefix.push_back('>');
}
