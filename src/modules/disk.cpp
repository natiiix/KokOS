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
    }

    vecArgs.dispose();
}
