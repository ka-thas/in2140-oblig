## IN2140 oblig

Team: Christian, Iver, Ka

This is the only mandatory assignment for the spring '24 semester. A bunch of seminary teachers have been warning us of it's difficulty :/

struct inode* load_inodes();
The function reads the master file table file and creates an inode in memory for each
corresponding entry in the file. The function puts pointers between the inodes correctly.
The master file table file on disk remains unchanged.
If the loading operation succeeds, the inode returned by this function should always be
the root directory, meaning that its name field should point to the string “/”.