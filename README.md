# IN2140 oblig

> Team: Christian, Iver, Ka

## 2.1 Design

### How to read the master file table (as described in the lecture slides) file from disk and load the inodes into memory.

Just move the pointer through every field as visualized in this assignment. If a field has a variable length, like 'children' and 'name', it will explicitly say how many bytes it takes in 'num_children' and 'name_len'

This feels very similar to interpreting machine instructions bit by bit in IN2060.

Load_inodes():
- Save a pointer to a struct inode
- Read the first 4 bytes of the MFT and save the read data to the inode struct's int ID.
- Temporarily save the next 4 bytes in a int name_len
- Read the next 'name_len' bytes and save them to a char pointer
- Read one byte and save it to the struct's is_directory flag

If the flag is true (dir inode):
- set the struct inode's 'blocks' pointer to NULL
- Read four bytes and save them to the struct's num_children
- Allocate space in memory for 'num_children' struct inode pointer-to-pointers. Call it temp_children
- In the MFT pointer, skip 'num_childre' * 8 bytes
- 'num_children' times: recursively call load_inodes() and assign the returned inode pointers to the temp_children array of pointers.
- Assign the temp_children array to the inode's children field.

If the flag is false (file inode):
- read int filesize
- read int num_blocks
- read 'num_blocks'*8 bytes to size_t blocks array

In this recursive implementation, the base cases are the leaf nodes files and empty directories.

### Any implementation requirements that are not met.

### Any part of the implementation that deviates from the precode. For example, if you are creating your own files, explain what their purpose is.

### Any tests that fail and what you think the cause may be