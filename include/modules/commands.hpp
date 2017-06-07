#pragma once

class string;

void cmd_help(const string& strArgs);
void cmd_cd(const string& strArgs);
void cmd_dir(const string& strArgs);
void cmd_mkfile(const string& strArgs);
void cmd_mkdir(const string& strArgs);
void cmd_delete(const string& strArgs);
void cmd_copy(const string& strArgs);
void cmd_move(const string& strArgs);
void cmd_rename(const string& strArgs);
void cmd_disk(const string& strArgs);
void cmd_color(const string& strArgs);

void cmd_text(const string& strArgs);
void cmd_exec(const string& strArgs);
