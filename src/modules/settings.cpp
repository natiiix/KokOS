#include <stdint.h>
#include <drivers/storage/fat.h>
#include <kernel.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#define STRING_FILE_PATH "system.cfg"

#define STRING_SHELL_BG "ShellBackground"
#define STRING_SHELL_FG "ShellForeground"
#define STRING_TEXT_EDITOR_BG "TextEditorBackground"
#define STRING_TEXT_EDITOR_FG "TextEditorForeground"

namespace Settings
{
    uint8_t Shell_Background;
    uint8_t Shell_Foreground;

    uint8_t TextEditor_Background;
    uint8_t TextEditor_Foreground;
    
    void loadDefault(void)
    {
        // Black background
        Shell_Background = 0x0;
        // Grey foreground
        Shell_Foreground = 0x7;

        // Same as terminal
        TextEditor_Background = 0x0;
        TextEditor_Foreground = 0x7;
    }

    bool parseColor(const string& str, uint8_t* const colorPtr)
    {
        int32_t tmp = 0;

        // Try to parse the value into an integer
        if (str.parseInt32(&tmp))
        {
            // The color value must a number from 0 to 15
            if (tmp >= 0 && tmp <= 15)
            {
                // Copy the color value
                (*colorPtr) = (uint8_t)tmp;
                // Success
                return true;
            }
        }
    
        // Failed to parse the value from the string
        return false;
    }

    void load(void)
    {
        // Load default settings (used when file doesn't exist / doesn't contain the setting)
        loadDefault();

        // Find the settings file
        struct FILE* file = getFile(0, partArray[0].rootDirCluster, STRING_FILE_PATH);
        
        // Settings file exists
        if (file)
        {
            debug_print("settings.cpp | Settings::load() | Settings file was found!");

            // Read data from the file
            uint8_t* data = readFile(file);
            delete file;

            // Convert the data to a string object
            string strData;
            strData.push_back((char*)data);
            delete data;

            // Split the string into separate lines
            vector<string> lines = strData.split('\n', false);

            // Load individual settings
            size_t lineslen = lines.size();            
            for (size_t i = 0; i < lineslen; i++)
            {
                // Separate words on each line
                vector<string> lineWords = lines.at(i).split(' ', true);

                size_t wordCount = lineWords.size();

                // Empty line
                if (!wordCount)
                {
                    lineWords.dispose();
                    continue;
                }

                if (lineWords.at(0).compare(STRING_SHELL_BG) && wordCount == 2)
                {
                    // Try to parse the color value
                    if (!parseColor(lineWords.at(1), &Shell_Background))
                    {
                        // Parsing failed
                        debug_print("settings.cpp | Settings::load() | Invalid shell background color value!");
                    }
                }
                else if (lineWords.at(0).compare(STRING_SHELL_FG) && wordCount == 2)
                {
                    // Try to parse the color value
                    if (!parseColor(lineWords.at(1), &Shell_Foreground))
                    {
                        // Parsing failed
                        debug_print("settings.cpp | Settings::load() | Invalid shell foreground color value!");
                    }
                }
                else if (lineWords.at(0).compare(STRING_TEXT_EDITOR_BG) && wordCount == 2)
                {
                    // Try to parse the color value
                    if (!parseColor(lineWords.at(1), &TextEditor_Background))
                    {
                        // Parsing failed
                        debug_print("settings.cpp | Settings::load() | Invalid text editor background color value!");
                    }
                }
                else if (lineWords.at(0).compare(STRING_TEXT_EDITOR_FG) && wordCount == 2)
                {
                    // Try to parse the color value
                    if (!parseColor(lineWords.at(1), &TextEditor_Foreground))
                    {
                        // Parsing failed
                        debug_print("settings.cpp | Settings::load() | Invalid text editor foreground color value!");
                    }
                }
                // This line contains an entry the loading function wasn't able to recognize
                else
                {
                    debug_print("settings.cpp | Settings::load() | Settings file contains an invalid entry!");
                }

                lineWords.dispose();
            }

            lines.dispose();
        }
        // File doesn't exist
        else
        {
            debug_print("settings.cpp | Settings::load() | Settings file doesn't exist!");
        }
    }

    void pushSetting(string& str, const char* const name, const int32_t value)
    {
        // Push the setting name
        str.push_back(name);
        str.push_back(' ');
        
        // Convert the numeric value to string
        string strValue = string::toString(value);

        // Push the value string to the base string
        str.push_back(strValue);

        strValue.dispose();

        // Put a new line at the end
        str.push_back('\n');
    }
    
    void save(void)
    {
        string strSettings;
        strSettings.clear();

        // Append all the settings to the settings string
        pushSetting(strSettings, STRING_SHELL_BG, Shell_Background);
        pushSetting(strSettings, STRING_SHELL_FG, Shell_Foreground);
        pushSetting(strSettings, STRING_TEXT_EDITOR_BG, TextEditor_Background);
        pushSetting(strSettings, STRING_TEXT_EDITOR_FG, TextEditor_Foreground);

        // Write the settings string to the settings file
        struct FILE* file = writeFile(0, partArray[0].rootDirCluster, STRING_FILE_PATH, (uint8_t*)strSettings.c_str(), strSettings.size());

        strSettings.dispose();

        if (!file)
        {
            debug_print("settings.cpp | Settings::save() | Failed to save the settings file!");
            return;
        }

        delete file;
    }
}
