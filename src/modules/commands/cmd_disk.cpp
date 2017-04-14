#include <drivers/storage/fat.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>
#include <c/stdio.h>
#include <c/string.h>
#include <modules/shell_global.hpp>

void cmd_disk_list_arguments(void)
{
    print("Valid arguments:\n");
    print("disks - lists detected hard disks\n");
    print("partitions - lists available FAT partitions\n");
}

void cmd_disk(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: disk <Argument>\n");
        cmd_disk_list_arguments();

        vecArgs.dispose();
        return;
    }
    if (vecArgs.at(0) == "disks")
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
    else if (vecArgs.at(0) == "partitions")
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

                char partLetter[2];
                partLetter[0] = 'A' + i;
                partLetter[1] = '\0';

                print("Partition #");
                printint(i);
                print(" [");
                print(&partLetter[0]);
                print("]: ");
                print(strpartinfo);
                print("\n");

                delete strpartinfo;
            }
        }
    }
    else
    {
        print("Invalid argument: \"");
        sprint(vecArgs.at(0));
        print("\"\n");
        cmd_disk_list_arguments();
    }

    vecArgs.dispose();
}
