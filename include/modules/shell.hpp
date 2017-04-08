#pragma once

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <modules/disk.hpp>

class Shell
{
public:
    void init(void);
    void process(const string& strInput);

private:
    string m_prefix;
    bool m_diskToolsEnabled; // false if there are no FAT partitions available
    uint8_t m_activePart;
    uint32_t m_activeDir; // first cluster of the active directory
    vector<string> m_pathDirs;

    // Modules
    Disk m_modDisk;

    // Internal shell calls
    void initModules(void);
    string readline(void);

    // Helping functions
    char* _generate_spaces(const size_t count);
    void _input_update(void);
    void _input_clean(void);
    void _update_prefix(void);
};
