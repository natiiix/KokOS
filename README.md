# KokOS
*very basic text-mode operating system developed by Ivo Meixner*

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

## In progress
- FAT file system support
- shell
- shell commands

## TODO
- file operations
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
