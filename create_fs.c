#include "inode.h"
#include "allocation.h"

#include <stdio.h>

int main( int argc, char* argv[] )
{
    if( argc != 3 )
    {
        fprintf( stderr, "Usage: %s MFT BAT\n"
                         "       where\n"
                         "       MFT is the name of the master_file_table\n"
                         "       BAT is the name of the block allocation table\n"
                         , argv[0] );
        exit( -1 );
    }

    char* mft_name = argv[1];
    char* bat_name = argv[2];

    set_block_allocation_table_name( bat_name );

    /* format_disk() makes sure that the simulated
     * disk is empty. It creates a file named
     * block_allocation_table that contains only
     * zeros. */
    format_disk();

    /* debug_disk() write the current content of the
     * block_allocation_table that simulates whether
     * blocks on disk contain file data (1) or not (0).
     */
    debug_disk();

    printf("===================================\n");
    printf("= Create root dir                 =\n");
    printf("===================================\n");
    struct inode* root = create_dir( NULL, "/" );
    debug_fs( root );

    printf("===================================\n");
    printf("= Create kernel file in dir root = \n");
    printf("===================================\n");
    create_file( root, "kernel", 20000 );
    debug_fs( root );
    debug_disk();

    printf("===================================\n");
    printf("= Create dir etc in dir root      =\n");
    printf("===================================\n");
    struct inode* dir_etc = create_dir( root, "etc" );
    debug_fs( root );
    create_file( dir_etc, "hosts", 200 );
    debug_fs( root );
    debug_disk();

    printf("===================================\n");
    printf("= Create usr/bin, usr/local/bin   =\n");
    printf("===================================\n");
    struct inode* dir_usr   = create_dir( root, "usr" );
    debug_fs( root );
    struct inode* dir_bin   = create_dir( dir_usr, "bin" );
    debug_fs( root );
    struct inode* dir_local = create_dir( dir_usr, "local" );
    debug_fs( root );
    struct inode* dir_lbin  = create_dir( dir_local, "bin" );
    debug_fs( root );
    create_file( dir_bin, "ls", 14322 );
    debug_fs( root );
    create_file( dir_bin, "ps", 13800 );
    debug_fs( root );
    create_file( dir_lbin, "nvcc", 28000 );
    debug_fs( root );
    create_file( dir_lbin, "gcc", 12623 );
    debug_fs( root );
    debug_disk();

    save_inodes( mft_name, root );

    fs_shutdown( root );

    release_block_allocation_table_name( );

    printf( "++++++++++++++++++++++++++++++++++++++++++++++++\n" );
    printf( "+ All inodes structures have been              +\n" );
    printf( "+ deleted. The inode info is stored in         +\n" );
    printf( "+ %s. The allocated file blocks +\n", mft_name );
    printf( "+ are stored in %s         +\n", bat_name );
    printf( "++++++++++++++++++++++++++++++++++++++++++++++++\n" );
}

