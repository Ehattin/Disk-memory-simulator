# Disk-memory-simulator
This program simulates how the OS accesses and manages the disk file system.
The disk file system simulates how the OS organizes and stores the data of a single folder in memory.
The folder can have multiple files in it - with a total storage capacity size of 256 characters (can be changed).
Every file on the disk has 3 Direct blocks of storage and a single indirect block of size blockSize to store additional data(in this case blocksize = 4 but it can be changed).
The total size that a file can contain in this case is 3*directblock*blocksize(Direct block) + blocksize*blocksize(single indirect block).


Compilation : Cntrl+Shift+B

Run: Ctrl + F5

Files: 
main.cpp - main file + main program
makefile
Readme
DISK_SIM_FILE.txt - file used as the disk memory for the simulation


required input: 
In order to use the program, user will have to input numbers 0-8 in order to activate the required actions:
0 - Deletes the entire disk and exits   
1 - Prints all of the files currently in the folder and their contents - empty folders will simply have no name
2 - Formats the disk - user must then input the block numbers and the direct entries he wants to use every file in the disk
3 - Creates a file - user must then input the filename he wants. The function will return the fd of the file.
4 - Opens a file - user must input the filename that he wishes to open. the file must already exist.
5 - Closes a file - user must input the file fd the he wishes to close.
6 - Writes data to the file - User must input the fd of the file and then the data he wishes to write to the file. 
7 - Reads the data from the file - user must input the fd of the file to read.
8 - Deletes a file - user must input the fd he wishes to remove from the disk, the program will remove the file name from the vector and mark it as empty but the
    old data will still remain (when user inputs new data the old data will be overloaded)
