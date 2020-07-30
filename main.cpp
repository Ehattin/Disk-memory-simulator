//Elchanan Hattin ID 320515927


#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

void decToBinary(int n, char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

// #define SYS_CALL
// ============================================================================
class fsInode
{
    int fileSize;
    int block_in_use;
    int *directBlocks;
    int singleInDirect;
    int num_of_direct_blocks;
    int block_size;

public:
    fsInode(int _block_size, int _num_of_direct_blocks)
    {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        num_of_direct_blocks = _num_of_direct_blocks;
        directBlocks = new int[num_of_direct_blocks];
        assert(directBlocks);
        for (int i = 0; i < num_of_direct_blocks; i++)
        {
            directBlocks[i] = -1;
        }
        singleInDirect = -1;
    }


    //large amount of setters and getters to update the various fields of the files
    int *getdirectBlockArr()
    {
        return directBlocks;
    }
    void setFileSize(int newFileSize)
    {
        fileSize = newFileSize;
    }
    int getFileSize()
    {
        return fileSize;
    }
    void setBlockInUse(int BlockInUse)
    {
        block_in_use = BlockInUse;
    }
    int getBlockInUse()
    {
        return block_in_use;
    }
    void setNumDirectBlocks(int NumDirectBlocks)
    {
        num_of_direct_blocks = NumDirectBlocks;
    }
    int getNumDirectBlocks()
    {
        return num_of_direct_blocks;
    }
    void setSingleIndirect(int SingleIndirect)
    {
        singleInDirect = SingleIndirect;
    }
    int getSingleIndirect()
    {
        return singleInDirect;
    }
    void setBlockSize(int BlockSize)
    {
        block_size = BlockSize;
    }
    int getBlockSize()
    {
        return block_size;
    }

    ~fsInode()
    {
        delete directBlocks;
    }
};

// ============================================================================
class FileDescriptor
{
    pair<string, fsInode *> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode *fsi)
    {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
    }

    string getFileName()
    {
        return file.first;
    }

    fsInode *getInode()
    {

        return file.second;
    }

    bool isInUse()
    {
        return (inUse);
    }
    void setInUse(bool _inUse)
    {
        inUse = _inUse;
    }
    void setFileName(string FileName)
    {
        file.first = FileName;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk
{
    FILE *sim_disk_fd;

    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode *> MainDir;

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector<FileDescriptor> OpenFileDescriptors;
    int direct_entries;
    int block_size;
    int totalBlocksUsed;

public:
    // ------------------------------------------------------------------------
    fsDisk()
    {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);
        for (int i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
        is_formated = false; //this creates an error otherwise
    }

    // ------------------------------------------------------------------------
    /*
    ListAll - Prints the disk contents
    Input - none
    Output - prints all of the files currently on the disk
    */
    void listAll()
    {
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            cout << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: " << it->isInUse() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    /*
    fsFormat - formats the disk with the amount of direct entries and blocksize that user wants
    Input - int blocksize, int direct entries
    Output - prints the number of blocks that user can have with given disk
    */
    void fsFormat(int blockSize = 4, int direct_Entries_ = 3)
    {
        totalBlocksUsed = 0;
        block_size = blockSize;
        direct_entries = direct_Entries_;
        BitVectorSize = DISK_SIZE / block_size; //cacl size of Bitvector
        BitVector = new int[BitVectorSize];
        assert(BitVector);
        memset(BitVector, 0, BitVectorSize); //init Bitvector
        is_formated = true;
        cout << "FORMAT DISK:  number of blocks " << BitVectorSize << endl;
    }

    // ------------------------------------------------------------------------
    /*
    Createfile - Creates a new file in the disk
    Input - String filename
    Output - prints the file that was created with its given fd if success, else -1
    */
    int CreateFile(string fileName)
    {
        if(findFreeBlock() ==-1){
            cout << "No free blocks available, cannot create a new file" << endl;
            return -1;    
        }
        if (is_formated == false)
        {
            cout << "Disk is not formatted" << endl;
            return -1;
        }
        fsInode *node = new fsInode(block_size, direct_entries); //new node
        assert(node);
        if (MainDir.count(fileName) > 0)
        {
            cout << "a file with this name already exists" << endl;
            return -1;
        }
        FileDescriptor file(fileName, node);       //create new file
        OpenFileDescriptors.push_back(file);       //add file to the vector list
        MainDir.insert(make_pair(fileName, node)); //add to maindir
        return OpenFileDescriptors.size() - 1;
    }

    // ------------------------------------------------------------------------
    /*
    OpenFile - Opens a file
    Input - String Filename
    Output - prints the file that was opened with given fd if success, else -1
    */
    int OpenFile(string fileName)
    {
        if (is_formated == false)
        {
            cout << "Disk is not formatted" << endl;
            return -1;
        }
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            if (it->getFileName().compare(fileName) == 0)
            { // check if the fileName exists in the vector
                if (it->isInUse() == true)
                { // check if file has already been opened
                    cout << "file already opened!" << endl;
                    return -1;
                }
                else
                {                       //file exists but isnt open
                    it->setInUse(true); //update the file to be in use
                    return i;
                }
                i++;
            }
        }
        // filename doesnt exist in the vector, cant open!
        cout << "Cannot open the file, doesn't exist!" << endl;
        return -1;
    }

    // ------------------------------------------------------------------------
    /*
    CloseFile - Closes an open file
    Input - int fd
    Output - prints the file that was closed with given fd if success, else -1
    */
    string CloseFile(int fd)
    {
        if (is_formated == false)
        {
            cout << "Disk is not formatted" << endl;
            return "-1";
        }
        if (fd <= OpenFileDescriptors.size() - 1)
        {
            if (OpenFileDescriptors[fd].isInUse() == true)
            { //check if the file is open ,and close the file
                OpenFileDescriptors[fd].setInUse(false);
                return OpenFileDescriptors[fd].getFileName();
            }
            else
            { //(OpenFileDescriptors[fd].isInUse() == false)
                cout << "The file has already been closed!" << endl;
                return "-1";
            }
        }
        else
        {
             // filename doesnt exist in the vector, cant close!
            cout << "Cannot close the file, doesn't exist!" << endl;
            return "-1";
        }
    }

    // ------------------------------------------------------------------------
    /*
    WriteToFile - Writes to a given file
    Input - int fd, the characters you wish to write, and their total length.
    Output - none (-1 if error)
    */
    int WriteToFile(int fd, char *buf, int len)
    {
        if (is_formated == false)
        {
            cout << "Disk is not formatted" << endl;
            return -1;
        }
        fsInode *node = OpenFileDescriptors[fd].getInode();
        int maxFileSize = (direct_entries + block_size) * block_size;
        int blocksNeeded = len / block_size; //amount of blocks needed to write the given file
        if (len % block_size != 0)
            blocksNeeded++;
        int freeBlocks = BitVectorSize - totalBlocksUsed; //check how many free blocks we can allocate for a given file
        if (is_formated == false)
        { //check if disk has been formatted
            cout << "Disk is not formatted" << endl;
            return -1;
        }
        if (len > maxFileSize - node->getFileSize())
        {
            cout << "Not enough disk space available to write given file" << endl;
            return -1;
        }
        if (fd > OpenFileDescriptors.size() - 1)
        {
            cout << "file not in FileDescriptor Vector" << endl;
            return -1;
        }

        if (OpenFileDescriptors[fd].isInUse() == false)
        { //check if the file is open and write
            cout << "Unable to write, the file has not been opened yet!" << endl;
            return -1;
        }

        if (blocksNeeded > freeBlocks)
        {
            cout << "Not enough blocks to allocate!" << endl;
            return -1;
        }

        int blockFrag = node->getBlockInUse() * block_size - node->getFileSize(); // free space within the blocks of a given file
        int blocksFound = 0;
        int i = 0; //this is the buf from the user (counter of characters)
        int *directB = node->getdirectBlockArr(); //this is the direct block array we use to save location of the direct blocks
        int freeSpaceLeft = len; //equal to the minimun between the emtpy cells in a given block and len of the required write chars
        if (len > blockFrag)
            freeSpaceLeft = blockFrag;
        int j = 0; //this is the blocks (counter of blocks)
        int k = 0; //this is the counter for the single blocks
        bool checker = false;
        int Sblock;
        while (i < len)
        { //i -> free space and free space -> len
            if (blockFrag > 0) //do we have free space?
            {

                if (node->getBlockInUse() > direct_entries)
                { //in the single indirect
                char b; //for our conversions
                int calc = 0;
                int singleOffSet = node->getBlockInUse()- direct_entries; //what block within the single indirects are we in?
                int ret_val = fseek(sim_disk_fd,node->getSingleIndirect()*block_size+singleOffSet, SEEK_SET); //go to the single block+offset
                ret_val = fread(&b, 1, 1, sim_disk_fd);
                assert(ret_val == 1);
                calc = (int)b;
                ret_val = fseek(sim_disk_fd,(calc*block_size)+(block_size-freeSpaceLeft), SEEK_SET); //go to the single block+offset
                ret_val = fwrite(buf, 1, freeSpaceLeft, sim_disk_fd);
                if (freeSpaceLeft == len)
                    {                                     //if we managed to write the entire amount of len to the direct blocks
                        node->setFileSize(freeSpaceLeft); //add the correct size
                        return 1;
                    }
                }
                else
                { //in the direct blocks

                    int ret_val = fseek(sim_disk_fd, (directB[node->getBlockInUse() - 1] * block_size) + (block_size - blockFrag), SEEK_SET); //go to last block + offset
                    ret_val = fwrite(buf, 1, freeSpaceLeft, sim_disk_fd);
                    assert(ret_val == 1);
                    if (freeSpaceLeft == len)
                    {   //if we managed to write the entire amount of len to the direct blocks
                        node->setFileSize(freeSpaceLeft); //add the correct size
                        return 1;
                    }
                    i += freeSpaceLeft;
                    blockFrag = 0; //now theres no emtpy space left in the block
                }
            }

            // no more cells in given block
            if (node->getBlockInUse() > direct_entries)//now check if we are within the single  indirect blocks
            { 
            int SB = findFreeBlock(); // this is the single block indirect we found that we are going to use to write the additional data
            node->setBlockInUse(node->getBlockInUse() +1);//update the num of blocks in use
            char c = '\0';
            if(!checker){
                Sblock = findFreeBlock();
                node->setSingleIndirect(Sblock);
                checker = true; //first time going into the single block
            }
            while (j < block_size) //same as the direct blocks
                {
                    int ret_val = fseek(sim_disk_fd, (SB * block_size) + (j), SEEK_SET); //go to last block + offset
                    ret_val = fwrite(buf + i, 1, 1, sim_disk_fd); //we write ONE character from the buf
                    assert(ret_val == 1);
                    j++;
                    i++;
                    if (i == len)
                    { //we got to the end of the buf
                        node->setFileSize(node->getFileSize() + len);
                        decToBinary(SB, c);
                        fseek(sim_disk_fd, Sblock*block_size+k, SEEK_SET);
                        fwrite(&c,1,1,sim_disk_fd);
                        return 1;
                    }
                }
            j = 0;
            decToBinary(SB, c);
            fseek(sim_disk_fd, Sblock*block_size+k, SEEK_SET); // this time k is the offset within the single block
            fwrite(&c,1,1,sim_disk_fd);
            k++;

            }
            else
            { //in the direct blocks
                cout << node->getFileSize() << endl;
                directB[node->getBlockInUse()] = findFreeBlock();
                node->setBlockInUse(node->getBlockInUse() + 1);
                while (j < block_size)
                {
                    int ret_val = fseek(sim_disk_fd, (directB[node->getBlockInUse()-1] * block_size) + (j), SEEK_SET); //go to last block + offset
                    ret_val = fwrite(buf + i, 1, 1, sim_disk_fd); //we write ONE character from the buf
                    assert(ret_val == 1);
                    j++;
                    i++;
                    if (i == len)
                    { //we got to the end of the buf
                        node->setFileSize(node->getFileSize() + len);
                        return 1;
                    }
                }
                    j = 0;
            }
        }
    }

    // ------------------------------------------------------------------------
    /*
    ReadFromFile - reads from a given file
    Input - int fd, and the length of the characters wished to read
    Output - the contents of the file with the length the user specified
    */
    int ReadFromFile(int fd, char *buf, int len)
    {
        if (is_formated == false)
        { //check if disk has been formatted
            cout << "Disk is not formatted" << endl;
            return -1;
        }
        if (fd > OpenFileDescriptors.size() - 1)
        {
            cout << "file not in FileDescriptor Vector" << endl;
            return -1;
        }

        if (OpenFileDescriptors[fd].isInUse() == false)
        { //check if the file is open and read
            cout << "Unable to read, the file has not been opened!" << endl;
            return -1;
        }

        fsInode *node = OpenFileDescriptors[fd].getInode();
        memset(buf, 0, DISK_SIZE); //every run we need to start from a fresh read
        int *directB = node->getdirectBlockArr();
        int minSizeOf = len; //saves the mininum size between the length of the file to read and the length of the fileSize currently used
        if (len > node->getFileSize())
            minSizeOf = node->getFileSize();
        int i = 0, j = 0, k = 0, l=0; //i = characters from len, j = blocks, k = indexes within the blocks, l = index of the single block
        
        while (i < minSizeOf)
        { 
            if (i <= (direct_entries * block_size))
            { //direct entries only, not single indirects 
                while (j < block_size)
                {
                    
                    int ret_val = fseek(sim_disk_fd, (directB[k] * block_size) + (j), SEEK_SET); //go to last block + offset
                    ret_val = fread(buf + i, 1, 1, sim_disk_fd);        //we read ONE character from the buf
                    assert(ret_val == 1);
                    j++;
                    i++;
                    if (i == minSizeOf) //we got to the end of the buf
                        return 1;
                }
                k++;
                j = 0;
            }
            else{//we are in a single indirect block 
                while (j < block_size) // go through the block
                {
                    char a = '\0';
                    int ret_val = fseek(sim_disk_fd, (node->getSingleIndirect()*block_size) + (l), SEEK_SET); //go to last block + offset
                    ret_val = fread(&a, 1, 1, sim_disk_fd); //we read ONE character from the buf
                    assert(ret_val == 1);
                    int res =0;
                    res = (int)a;
                    ret_val = fseek(sim_disk_fd, (res*block_size)+j, SEEK_SET);
                    ret_val = fread(buf+i, 1, 1 , sim_disk_fd);
                    j++;
                    i++;
                    if (i == minSizeOf) //we got to the end of the buf
                        return 1;
                }
                l++;
                j = 0;
            }
            }
        }

    // ------------------------------------------------------------------------
    /*
    DelfFile - deletes a file, removes all of the pointers and erases the name but - if the file contained data it will simply be overwritten next time its allocated to a file
    Input - String fileName
    Output - int of the file fd deleted
    */
    int DelFile(string FileName)
    {
        if (is_formated == false)
        { //check if disk has been formatted
            cout << "Disk is not formatted" << endl;
            return -1;
        }

        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            if (it->getFileName().compare(FileName) == 0)
            {
                if(it->isInUse() == true){
                cout << "Cant delete an open file!" << endl;
                return -1;    
                } // check if the fileName exists in the vector and erase it
                int *arr = it->getInode()->getdirectBlockArr(); //this arr is the direct block arr of the given file
                for (int j = 0; j < direct_entries; j++)
                {
                    BitVector[arr[j]] = 0;
                } //this erases the direct blocks that the file was using
                OpenFileDescriptors[i].setInUse(false);
                OpenFileDescriptors[i].setFileName("");
                auto iter = MainDir.find(FileName);
                delete iter->second;
                MainDir.erase(iter);
                return i;
            }
            i++;
        }
        
        // filename doesnt exist in the vector, cant delete!
        cout << "Cannot Delete the file, it doesn't exist!" << endl;
        return -1;
    }

    // ------------------------------------------------------------------------
    /*
    Destructor!!!
    */
    ~fsDisk(){
        fclose(sim_disk_fd);
        delete[] BitVector;
        MainDir.clear();
        OpenFileDescriptors.clear();
    }


    // ------------------------------------------------------------------------
    /*
    FindFreeBlock searchers for a free block in the Bitvector
    returns the location of the free block or -1 if there are no free blocks left
    */
    int findFreeBlock()
    {
        for (int i = 0; i < BitVectorSize; i++)
        {
            if (BitVector[i] == 0)
            {
                BitVector[i] = 1;
                totalBlocksUsed ++;
                return i;
            }
        }
        return -1;
    }
};

int main()
{
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while (1)
    {
        cin >> cmd_;
        switch (cmd_)
        {
        case 0: // exit
            delete fs;
            exit(0);
            break;

        case 1: // list-file
            fs->listAll();
            break;

        case 2: // format
            cin >> blockSize;
            cin >> direct_entries;
            fs->fsFormat(blockSize, direct_entries);
            break;

        case 3: // creat-file
            cin >> fileName;
            _fd = fs->CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 4: // open-file
            cin >> fileName;
            _fd = fs->OpenFile(fileName);
            cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 5: // close-file
            cin >> _fd;
            fileName = fs->CloseFile(_fd);
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 6: // write-file
            cin >> _fd;
            cin >> str_to_write;
            fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));
            break;

        case 7: // read-file
            cin >> _fd;
            cin >> size_to_read;
            fs->ReadFromFile(_fd, str_to_read, size_to_read);
            cout << "ReadFromFile: " << str_to_read << endl;
            break;

        case 8: // delete file
            cin >> fileName;
            _fd = fs->DelFile(fileName);
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;
        default:
            break;
        }
    }
}