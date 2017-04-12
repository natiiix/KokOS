#include <modules/disk.hpp>
#include <drivers/storage/fat.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>
#include <c/stdio.h>
#include <c/string.h>
#include <modules/shell_global.hpp>

void Disk::process(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);

    if (vecArgs.size() > 0)
    {
        if (vecArgs.at(0) == "list" && vecArgs.size() == 2)
        {
            if (vecArgs.at(1) == "disks")
            {
                if (hddCount == 0)
                {
                    print("No hard disks found.\n");
                }
                else
                {
                    for (size_t i = 0; i < hddCount; i++)
                    {
                        char* strdiskinfo = getHddInfoStr(i);

                        print("Disk #");
                        printint(i);
                        print(": ");
                        print(strdiskinfo);
                        print("\n");

                        delete strdiskinfo;
                    }
                }
            }
            else if (vecArgs.at(1) == "partitions")
            {
                if (partCount == 0)
                {
                    print("No partitions found.\n");
                }
                else
                {
                    for (size_t i = 0; i < partCount; i++)
                    {
                        char* strpartinfo = getPartInfoStr(i);

                        print("Partition #");
                        printint(i);
                        print(": ");
                        print(strpartinfo);
                        print("\n");

                        delete strpartinfo;
                    }
                }
            }
        }
        else if (vecArgs.at(0) == "check" && vecArgs.size() == 2)
        {
            struct FILE* file = getFile(activePart, activeDir, vecArgs.at(1).c_str());

            if (file == nullptr)
            {
                print("Invalid file path!\n");
            }
            else
            {
                print(file->name);
                print(" - ");
                printint(file->size);
                print(" Bytes\n");

                char* content = (char*)readFile(file);

                if (content)
                {
                    content[file->size] = '\0'; // WARNING: this is actually outside the allocated memory boundaries
                    print(content);
                    delete content;
                }
                else
                {
                    print("Empty file.\n");
                }

                delete file;
            }
        }
        else
        {
            print("Invalid arguments!\n");
        }
    }

    vecArgs.dispose();
}
