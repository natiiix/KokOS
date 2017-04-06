#include <modules/disk.hpp>
#include <drivers/storage/fat.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>
#include <c/stdio.h>
#include <c/string.h>

void Disk::process(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);

    char* strsize = tostr(vecArgs.size(), 10);
    print(strsize);
    print("\n");
    free(strsize);

    if (vecArgs.size() > 0)
    {
        if (vecArgs[0] == "list\0" && vecArgs.size() > 1)
        {
            if (vecArgs[1] == "disks\0")
            {
                // TODO: list disks
                print("DISKS");
            }
            else if (vecArgs[1] == "partitions")
            {
                // TODO: list partitions
                print("PARTITIONS");
            }
        }
    }

    //string::disposeVector(vecArgs);

    for (size_t i = 0; i < vecArgs.size(); i++)
    {
        vecArgs[i].dispose();
    }

    vecArgs.dispose();
}
