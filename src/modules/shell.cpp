#include <modules/shell.hpp>

#include <c/stdlib.h>
#include <c/stdio.h>
#include <c/string.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <kernel.h>

#include <drivers/storage/fat.h>
#include <modules/commands.hpp>

namespace Shell
{
	uint8_t activePart; // index of the active partition
	uint32_t activeDir; // first cluster of the active directory

	string shellPrefix;
	bool diskToolsEnabled; // false if there are no FAT partitions available
	vector<string> pathStructure;

	vector<string> cmdHistory;
	uint8_t historyIdx;

	void process(const string& strInput)
	{
		// Extract command string from the input string
		string strCmd;

		size_t strsize = strInput.size();

		for (size_t i = 0; i < strsize && strInput.at(i) != ' '; i++)
		{
			// Read the command word, automatically convert it to lowercase
			strCmd.push_back(ctolower(strInput.at(i)));
		}

		string strArgs;

		// If the command itself isn't the entire content of the input string
		if (strCmd.size() + 1 < strsize)
		{
			// The old string must be properly disposed
			strArgs.dispose();

			// Separate arguments from command string
			strArgs = strInput.substr(strCmd.size() + 1);
		}

		// -- Check internal shell commands --
		// Displays available commands and their proper syntax
		if (strCmd == "help")
		{
			cmd_help(strArgs);
		}
		// Partition switch
		else if (strCmd.size() == 2 && strCmd[1] == ':')
		{
			if (strCmd[0] >= 'a' && strCmd[0] <= 'z' && // letters represent partitions index
				(strCmd[0] - 'a') < partCount) // check partition index validity
			{
				activePart = strCmd[0] - 'a'; // change the active partition
				pathStructure.clear(); // clear the path because it doesn't exist on this partition
				_update_prefix();
			}
			else
			{
				print("Invalid partition letter!\n");
			}
		}
		// Clears the terminal
		else if (strCmd == "clear" && strArgs.size() == 0)
		{
			clear();
		}
		// Directory switch
		else if (strCmd.compare("cd"))
		{
			cmd_cd(strArgs);
		}
		// List directory content
		else if (strCmd.compare("dir"))
		{
			cmd_dir(strArgs);
		}
		// Create a new file
		else if (strCmd.compare("mkfile"))
		{
			cmd_mkfile(strArgs);
		}
		// Create a new directory
		else if (strCmd.compare("mkdir"))
		{
			cmd_mkdir(strArgs);
		}
		// Delete a file/directory
		else if (strCmd.compare("delete"))
		{
			cmd_delete(strArgs);
		}
		else if (strCmd.compare("copy"))
		{
			cmd_copy(strArgs);
		}
		// -- Compare the input string against each module command string --
		// Disk operation module
		// Syntax: disk <Action> <Arguments>
		else if (strCmd.compare("disk"))
		{
			cmd_disk(strArgs);
		}
		else if (strCmd.compare("text"))
		{
			cmd_text(strArgs);
		}
		else if (strCmd.compare("exec"))
		{
			cmd_exec(strArgs);
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

	char* _generate_spaces(const size_t count)
	{
		char* strspaces = (char*)malloc(count + 1);
		strspaces[count] = '\0';
		
		for (size_t i = 0; i < count; i++)
		{
			strspaces[i] = ' ';
		}

		return strspaces;
	}

	void historyAppend(const string& strInput)
	{
		// Don't append empty string to the history vector
		if (!strInput.size())
		{
			return;
		}

		// Don't append the command to the history vector if it's equal to the last entry
		if (cmdHistory.size() && cmdHistory.back().compare(strInput))
		{
			return;
		}

		// Prepare the command string
		string strHistoryEntry;

		strHistoryEntry.clear();
		strHistoryEntry.push_back(strInput);

		// If the history vector size limit has been reached delete the oldest entry
		if (cmdHistory.size() >= HISTORY_LIMIT)
		{
			cmdHistory.remove(0);
		}

		// Push the command string to the history vector
		cmdHistory.push_back(strHistoryEntry);
	}

	void historyUp(string& strInput)
	{
		size_t cmdsize = cmdHistory.size();

		// There are no commands in the history vector
		if (!cmdsize)
		{
			return;
		}

		// Shell is in writing mode
		if (historyIdx == HISTORY_INDEX_DEFAULT)
		{
			// Move to the first history entry
			historyIdx = cmdsize - 1;
		}
		// If it's possible to go further in the history vector
		else if (historyIdx > 0)
		{
			// Move to an older history entry
			historyIdx--;
		}
		// The history index is already pointing at the oldest entry in the history vector
		else
		{
			return;
		}

		// Replace the current input string with the one from the history vector
		strInput.clear();
		strInput.push_back(cmdHistory.at(historyIdx));
	}

	void historyDown(string& strInput)
	{
		size_t cmdsize = cmdHistory.size();

		// There are no commands in the history vector
		if (!cmdsize)
		{
			return;
		}

		if (historyIdx == HISTORY_INDEX_DEFAULT)
		{
			return;
		}

		strInput.clear();

		// There's a more recent command entry in the history vector than the current one,
		// move the history index forward
		if (historyIdx < cmdsize - 1)
		{
			// Move to an more recent history entry
			historyIdx++;
		}
		// The currently selected history entry is the latest, switch to writing mode
		else
		{
			historyIdx = HISTORY_INDEX_DEFAULT;
			return;
		}

		// Display the currently selected command from the history vector
		strInput.push_back(cmdHistory.at(historyIdx));
	}

	string readline(void)
	{
		if (getcol() > 0)
		{
			newline();
		}

		size_t row = getrow();

		size_t preLen = shellPrefix.size();
		size_t inSpace = VGA_WIDTH - preLen - 1;

		string strInput;

		while (true)
		{
			struct keyevent ke = readKeyEvent();
				
			if (ke.state)
			{			
				if (ke.keychar > 0)
				{
					historyIdx = HISTORY_INDEX_DEFAULT;

					// Append a character to the input string
					strInput += ke.keychar;
				}
				else if (ke.scancode == KEY_ENTER && !ke.modifiers)
				{
					historyIdx = HISTORY_INDEX_DEFAULT;

					// Generate spaces to clear the input line on the screen
					char* strspaces = _generate_spaces(VGA_WIDTH);
					printat(strspaces, 0, row);
					delete strspaces;

					// Append the command string to the command history vector
					historyAppend(strInput);
					return strInput;
				}
				else if (ke.scancode == KEY_BACKSPACE && !ke.modifiers)
				{
					historyIdx = HISTORY_INDEX_DEFAULT;

					// Delete the last character in the input string
					if (strInput.size() > 0)
					{
						strInput.pop_back();
					}
				}
				else if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
				{
					historyIdx = HISTORY_INDEX_DEFAULT;

					// Clear the input string
					strInput.clear();
				}
				else if (ke.scancode == KEY_ARROW_UP && !ke.modifiers)
				{
					historyUp(strInput);
				}
				else if (ke.scancode == KEY_ARROW_DOWN && !ke.modifiers)
				{
					historyDown(strInput);
				}
			}

			size_t inRenderLen = (inSpace >= strInput.size() ? strInput.size() : inSpace);
			size_t inStartIdx = strInput.size() - inRenderLen;

			sprintat(shellPrefix, 0, row);

			// Don't attempt to print input if there is none
			if (strInput.size())
			{
				string strInputRender = strInput.substr(inStartIdx, inRenderLen);
				sprintat(strInputRender, preLen, row);
				strInputRender.dispose();
			}

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

	void _update_prefix(void)
	{
		shellPrefix.clear();

		if (diskToolsEnabled)
		{
			string activePath = string::join(pathStructure, '/', true);

			shellPrefix.push_back('A' + activePart);
			shellPrefix.push_back(':');
			shellPrefix.push_back(activePath);
			shellPrefix.push_back('/');

			activePath.dispose();
		}

		shellPrefix.push_back('>');
	}
}

extern "C"
void shell_init(void)
{
	// Used to make sure there is no memory leaking during kernel initialization
	debug_memusage();
	// Give the user a chance to see kernel initialization messages
	debug_pause();

    clear();

	Shell::shellPrefix = string();
	Shell::pathStructure = vector<string>();

	Shell::cmdHistory = vector<string>();
	Shell::historyIdx = Shell::HISTORY_INDEX_DEFAULT;

	Shell::diskToolsEnabled = (partCount > 0);

	if (Shell::diskToolsEnabled)
	{
		Shell::activePart = 0;
		Shell::activeDir = partArray[Shell::activePart].rootDirCluster;
		Shell::pathStructure.clear();
		Shell::_update_prefix();
	}
	else
	{
		debug_print("shell.cpp | shell_init() | Disk tools disabled!");
		Shell::shellPrefix.clear();
		Shell::shellPrefix.push_back('>');
	}

    while (true)
    {
		// Keep in mind that some memory is always allocated by the shell instance itself
		debug_memusage();
		
        string strInput = Shell::readline();

		sprint(Shell::shellPrefix);
        sprint(strInput);
		newline();

		Shell::process(strInput);
		
		strInput.dispose();
    }
}
