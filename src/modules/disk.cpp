#include <modules/disk.hpp>
#include <drivers/storage/fat.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>
#include <c/stdio.h>
#include <c/string.h>

void Disk::process(const string& strArgs)
{
    sprint(strArgs);
    print("\n");

    vector<string> vecArgs = strArgs.split(' ', true);

    printint(vecArgs.size());
    
    for (size_t i = 0; i < vecArgs.size(); i++)
    {
        sprint(vecArgs[i]);
        print("\n");
    }

    if (vecArgs.size() > 0)
    {
        if (vecArgs[0] == "list" && vecArgs.size() > 1)
        {
            if (vecArgs[1] == "disks")
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

    vecArgs.dispose();
}
