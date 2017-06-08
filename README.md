# FileSystem

An adapted file system using i-nodes as base and a virtual disk to simulate the data disk.

### Disk organization

- There are no directories
- Each file will reserve a block
- Block size = 2048 b
- Max disk = 102400 b
- Max Files = 48
- CRC is used to ckeck data consistency

### Disk map


              1 block                 | 1 block |     48 Block
--------------------------------------------------------------

    Superblock || imap || bmap || crc ||  Inode  ||  Data Block  
---------------------------------------------------------------


## Execution

Write commands

1 make
2 ./test

**If you want to change the function calls just change _test.c_**




_The creation of the virtual disk was provided by UC3M University_
