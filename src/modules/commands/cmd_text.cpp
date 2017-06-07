#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

void textEditorStart(const char* const filePath);

void cmd_text(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: text <File Path>\n");
        
        vecArgs.dispose();
        return;
    }

    textEditorStart(vecArgs.at(0).c_str());

    vecArgs.dispose();
}
