# KokOS
*very basic text-mode operating system developed by Ivo Meixner*

## Disclaimer
**KokOS is meant to be executed in virtual environment (such as VMWare Workstation or Oracle VirtualBox) only.**

**I do NOT take any responsibility whatsoever for any harm caused to your data or your hardware as you have been warned that the code may be dangerous.**

**By running the code on your machine it you implicitly agree with this statement and take full responsibility for the outcode.**

## Branches
- **master** - should always be relatively stable and safe to run (at your own risk!)
- **unstable** - the latest version that is known to run somewhat properly, but may contain potentially dangerous code (you should avoid running this version and remain particularly cautious when doing so)
- **dev** - meant for development only, often contains unstable code which may seriously harm your device upon execution (please NEVER run this code on a physical machine regardless of the circumstances)

## Features
- text mode screen output
- ~~standard keyboard input (using port polling)~~ (removed due to redundancy / obsolescence)
- keyboard input using interrupts
- simple memory management (malloc, free,..)
- basic cstring operations
- partial C &lt;stdlib.h&gt; implementation
- partial C &lt;stdio.h&gt; implementation
- C++ &lt;string&gt; implementation
- C++ &lt;vector&gt; implementation
- basic shell commands (help, cd, dir,..)
- automatically converts paths to uppercase (because FAT doesn't seem to store lowercase names in basic directory entries)
- automatically converts commands to lowercase
- shell supports both absolute and relative path addressing

## In progress
- FAT file system support
- shell
- shell commands
- file operations (create, delete, read, write)
- directory operations (create, delete - a content check before deletion hasn't been implemented yet)

## TODO
- improve the shell help command (some information may not be up-to-date at the moment)
- OS-specific programming language (something along the lines of bash script)
- implement more C/C++ libraries
- shell command history (using up arrow)
- moving shell cursor using arrow keys (to allow command edits without having to delete portions of the entered command)
- fix hazardous user inputs
- basic text editor

## On Hold
- AHCI driver (I wasn't able to make it work and it's not of high importance to me)

## Links
<div>Inspiration / based upon: <a href="http://wiki.osdev.org/Main_Page">OSDev Wiki</a></div>
