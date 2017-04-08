#include <modules/disk.hpp>
#include <drivers/storage/fat.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>
#include <c/stdio.h>
#include <c/string.h>

void Disk::process(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);

    if (vecArgs.size() > 0)
    {
        if (vecArgs[0] == "list" && vecArgs.size() == 2)
        {
            if (vecArgs[1] == "disks")
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
            else if (vecArgs[1] == "partitions")
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
        else if (vecArgs[0] == "check" && vecArgs.size() == 3)
        {
            uint8_t partIdx = (uint8_t)strparse(vecArgs[1].c_str(), 10);
            if (partIdx >= partCount)
            {
                print("Invalid partition index!\n");
            }
            else
            {
                struct FILE* file = getFile(partIdx, vecArgs[2].c_str());

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

                    char* content = (char*)fatReadFile(file);

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
        }
        else
        {
            print("Invalid arguments!\n");
        }
    }

    vecArgs.dispose();
}
