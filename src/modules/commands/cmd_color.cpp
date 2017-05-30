#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <modules/settings.hpp>

void cmd_color(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 2)
    {
        print("Invalid arguments!\n");
        print("Syntax: color <Background Color> <Foreground Color>\n");
        
        vecArgs.dispose();
        return;
    }

    int32_t colorBG = 0;
    int32_t colorFG = 0;

    // Prase background color
    if (!vecArgs.at(0).parseInt32(&colorBG))
    {
        print("Invalid background color value!");
        vecArgs.dispose();
        return;
    }
    else if (colorBG < 0 || colorBG > 15)
    {
        print("Background color must be a number between 0 and 15!");
        vecArgs.dispose();
        return;
    }

    // Prase foreground color
    if (!vecArgs.at(1).parseInt32(&colorFG))
    {
        print("Invalid foreground color value!");
        vecArgs.dispose();
        return;
    }
    else if (colorFG < 0 || colorFG > 15)
    {
        print("Foreground color must be a number between 0 and 15!");
        vecArgs.dispose();
        return;
    }

    // Background color must NOT be the same as the foreground color
    if (colorBG == colorFG)
    {
        print("Background color and foreground color must not be the same!");
        vecArgs.dispose();
        return;
    }

    // Update the shell color scheme
    Settings::Shell_Background = (uint8_t)colorBG;
    Settings::Shell_Foreground = (uint8_t)colorFG;

    // Store the new shell color settings into the settings file
    Settings::save();

    vecArgs.dispose();
}
