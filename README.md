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
- **dev_xxx** - sub-branch of dev with a specified development topic waiting to be pulled to unstable, used to separate bigger chunks of changes done on the dev branch without having to modify the unstable branch

## Features
- simple text-mode shell interface
- ~~standard keyboard input (using port polling)~~ (removed due to redundancy / obsolescence)
- keyboard input using interrupts
- simple memory management (malloc, free,..)
- basic cstring operations
- partial C &lt;stdlib.h&gt; implementation
- partial C &lt;stdio.h&gt; implementation
- C++ &lt;string&gt; implementation
- C++ &lt;vector&gt; implementation
- basic shell commands (help, clear)
- basic file/directory operation commands (mkfile, mkdir, delete)
- prevents you from deleting a non-empty directory
- shell supports both absolute and relative path addressing
- shell command history (using up/down arrows, up to 16 commands)
- currently up-to-date help command
- repetitive validity checks should prevent unexpected behavior

## In progress
- shell
- FAT file system support
- more complex file operations
- basic text file editor (selections and copy-pasting may be implemented later)

## TODO
- OS-specific programming language (something along the lines of bash script)
- implement more C/C++ libraries
- moving shell cursor using arrow keys (to allow command edits without having to delete portions of the entered command)

## On Hold
- AHCI driver (I wasn't able to make it work and it's not of high importance to me)

## Links
<div>Inspiration / based upon: <a href="http://wiki.osdev.org/Main_Page">OSDev Wiki</a></div>
