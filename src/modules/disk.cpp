#include <modules/disk.hpp>
#include <drivers/storage/fat.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>
#include <c/stdio.h>
#include <drivers/storage/fat.h>

void Disk::process(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);

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
                
            }
        }
    }

    vecArgs.dispose();
}
