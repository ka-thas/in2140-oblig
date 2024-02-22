# IN2140 oblig

Team: Christian, Iver, Ka

This is the only mandatory assignment for the spring '24 semester. A bunch of seminary teachers have been warning us of it's difficulty :/

## Advice

It may be appropriate to implement the functions in this order:

1. Functions required for loading:

    – load_inodes

    – find_inode_by_name

2. Functions required to create files and directories and update the master file
   table and the block allocation table.

    – create_dir

    – create_file

    – fs_shutdown

3. Functions required to delete files and directories:

    – delete_file

    – delete_dir

## Implementation

[ ] Create file

[ ] Create Directory

[ ] Delete a file

[ ] Delete a directory

[ ] Find Inode by Name

[ ] Load File System

[ ] Shut Down File System

---

struct inode\* load_inodes();
The function reads the master file table file and creates an inode in memory for each
corresponding entry in the file. The function puts pointers between the inodes correctly.
The master file table file on disk remains unchanged.
If the loading operation succeeds, the inode returned by this function should always be
the root directory, meaning that its name field should point to the string “/”.
