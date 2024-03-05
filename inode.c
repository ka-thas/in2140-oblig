#include "allocation.h"
#include "inode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* The number of bytes in a block.
 * Do not change.
 */
#define BLOCKSIZE 4096

// The lowest unused node ID.
static int num_inode_ids = 0;

/* This helper function computes the number of blocks that you must allocate
 * on the simulated disk for a give file system in bytes. You don't have to use
 * it.
 * Do not change.
 */
static int blocks_needed(int bytes)
{
    int blocks = bytes / BLOCKSIZE;
    if (bytes % BLOCKSIZE != 0)
        blocks += 1;
    return blocks;
}

/* This helper function returns a new integer value when you create a new inode.
 * This helps you do avoid inode reuse before 2^32 inodes have been created. It
 * keeps the lowest unused inode ID in the global variable num_inode_ids.
 * Make sure to update num_inode_ids when you have loaded a simulated disk.
 * Do not change.
 */
static int next_inode_id()
{
    int retval = num_inode_ids;
    num_inode_ids += 1;
    return retval;
}

/* Oppretter en fil. */
struct inode *create_file(struct inode *parent, char *name, int size_in_bytes)
{
    if(!parent)return NULL;
    printf(":%s:\n", parent->name);
    if (find_inode_by_name(parent, name) != NULL)
    {
        return NULL;
    }

    // allocation test is run before all other variables are set, making freeing resources easier if it fails.
    int amount_of_blocks = blocks_needed(size_in_bytes);
    size_t *blockarr = malloc(amount_of_blocks * sizeof(size_t));
    if (blockarr == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for blockarr\n");
        return NULL;
    }
    for (int i = 0; i < amount_of_blocks; i++)
    {
        int number = allocate_block();
        if (number >= 0)
        {
            blockarr[i] = number;
        }
        else
        {
            format_disk();
            return NULL;
        }
    }

    struct inode *inode = malloc(sizeof(struct inode));
    if (inode == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for inode\n");
        return NULL;
    }

    inode->id = next_inode_id();
    inode->name = strdup(name);
    inode->is_directory = 0;
    inode->filesize = size_in_bytes;
    inode->blocks = blockarr;
    inode->num_blocks = amount_of_blocks;
    inode->children = NULL;

    // updating parent inode for all except root
    if (parent != NULL)
    {

        parent->num_children++;
        parent->children = realloc(parent->children, parent->num_children * sizeof(struct inode *));
        parent->children[parent->num_children - 1] = inode;
    }

    return inode;
}

struct inode *create_dir(struct inode *parent, char *name)
{
    if (parent != NULL)
    {
    if (find_inode_by_name(parent, name) != NULL) // if name already exists
    {
        return NULL;
    }

    struct inode *dir = malloc(sizeof(struct inode));
    if (dir == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for inode\n");
        return NULL;
    }


    dir->blocks = NULL;
    if (parent != NULL){
        parent->num_children++;
        int num_siblings = parent->num_children;
        parent->children = realloc(parent->children, parent->num_children * sizeof(struct inode *));
        parent->children[num_siblings - 1] = dir;
    }

    dir->id = next_inode_id();
    dir->name = strdup(name);
    dir->is_directory = 1;
    dir->num_children = 0;
    dir->children = NULL;

    return dir;
}

/* Check all the inodes that are directly referenced by
 * the node parent. If one of them has the name "name",
 * its inode pointer is returned.
 * parent must be directory.
 */
struct inode *find_inode_by_name(struct inode *parent, char *name)
{
    if (parent == NULL || parent->is_directory == 0)
    {
        fprintf(stderr, "Parent is NULL or not a directory\n");
        return NULL;
    }

    int num_children = parent->num_children;

    struct inode *child = NULL;
    for (int i = 0; i < num_children; i++)
    {
        child = parent->children[i];
        char* childname = child->name;
        if (strcmp(childname, name) == 0)
        {
            return child;
        }
    }

    return NULL;
}

static int verified_delete_in_parent(struct inode *parent, struct inode *node)
{
    int num_children = parent->num_children;
    for (int i = 0; i < num_children; i++)
    {
        if (parent->children[i] == node)
        {
            return 1;
        }
    }
    return 0;
}

int is_node_in_parent(struct inode *parent, struct inode *node)
{
    int i = parent->num_children;
    for (int j = 0; j < i; j++)
    {
        if (parent->children[j] == node)
        {
            return 1;
        }
    }
    return 0;
}

int delete_file(struct inode *parent, struct inode *node)
{

    for (int i = 0; i < node->num_blocks; i++)
    {
        free_block(node->blocks[i]);
    }

    struct inode **children = malloc(sizeof(struct inode *) * (parent->num_children - 1));
    for (int i = 0; i < parent->num_children; i++)
    {
        if (node == parent->children[i])
            continue;
        children[i] = parent->children[i];
    }
    parent->num_children--;
    parent->children = children;

    if (verified_delete_in_parent(parent, node) == 0)
    {
        fprintf(stderr, "Failed to delete node from parent\n");
        return -1;
    }
    free(node->name);
    free(node->blocks);
    free(node);
    return 0;
}

int delete_dir(struct inode *parent, struct inode *node)
{
    if (node->num_children != 0)
    {
        return -1;
    }

   
    /*
    parent->num_children--;
    parent->children[parent->num_children] = NULL;
    parent->children = realloc(parent->children, sizeof(struct inode) * parent->num_children);
    */

    if(parent != NULL){

    parent->num_children--;
    struct inode** tempchild;
    tempchild = malloc(sizeof(struct inode*)*parent->num_children);
    int j = 0;
    for (int i= 0; i<parent->num_children+1;i++){
        if(parent->children[i] == node ) continue;
        
        tempchild[j] = parent->children[i];
        j++;
    }    
    free(parent->children);
    parent->children = tempchild;
    }
    free(node->children);
    free(node->name);
    free(node->children);
    free(node);
    return 0;
}

struct inode *load_inodes_recursive(FILE *file, int *reader)
{
    next_inode_id();

    // prep
    struct inode *inode = malloc(sizeof(struct inode));
    if (inode == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for inode\n");
        return NULL;
    }
    fseek(file, *reader, SEEK_SET);

    // ID
    int id;
    fread(&id, sizeof(int), 1, file);
    inode->id = id;
    *reader += sizeof(int);

    // name_len;
    int name_len;
    fread(&name_len, sizeof(int), 1, file);
    *reader += sizeof(int);

    // name
    char *name_ptr = malloc(name_len);
    if (name_ptr == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for name\n");
        return NULL;
    }
    fread(name_ptr, 1, name_len, file);
    inode->name = name_ptr;
    *reader += sizeof(char) * name_len;

    // is_directory
    char is_directory;
    fread(&is_directory, sizeof(char), 1, file);
    inode->is_directory = is_directory;
    *reader += sizeof(char);

    if (is_directory)
    {
        inode->blocks = NULL;

        int num_children;
        fread(&num_children, sizeof(int), 1, file);
        inode->num_children = num_children;
        *reader += sizeof(int);

        if (num_children == 0)
        {
            inode->children = NULL;
            return inode;
        }

        struct inode **children = malloc(sizeof(struct inode *) * num_children);
        if (children == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for children\n");
            return NULL;
        }
        *reader += sizeof(size_t) * num_children;

        for (int i = 0; i < num_children; i++)
        {
            children[i] = load_inodes_recursive(file, reader);
        }

        inode->children = children;
    }
    else
    {
        inode->children = NULL;

        // filesize
        int filesize;
        fread(&filesize, sizeof(int), 1, file);
        inode->filesize = filesize;
        *reader += sizeof(int);

        // num_blocks
        int num_blocks;
        fread(&num_blocks, sizeof(int), 1, file);
        inode->num_blocks = num_blocks;
        *reader += sizeof(int);

        // blocks
        size_t *blocks = malloc(sizeof(size_t) * num_blocks);
        if (blocks == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for blocks\n");
            return NULL;
        }
        fread(blocks, sizeof(size_t), num_blocks, file);
        inode->blocks = blocks;
        *reader += sizeof(size_t) * num_blocks;
    }
    return inode;
}

/* Read the file master_file_table and create an inode in memory
 * for every inode that is stored in the file. Set the pointers
 * between inodes correctly.
 * The file master_file_table remains unchanged.
 */
struct inode *load_inodes(char *master_file_table)
{
    FILE *file = fopen(master_file_table, "r");

    if (!file)
    {
        fprintf(stderr, "Failed to open file %s\n", master_file_table);
        return NULL;
    }

    int reader = 0;
    struct inode *root = load_inodes_recursive(file, &reader);

    fclose(file);
    return root;
}

/* The function save_inode is a recursive functions that is
 * called by save_inodes to store a single inode on disk,
 * and call itself recursively for every child if the node
 * itself is a directory.
 */
static void save_inode(FILE *file, struct inode *node)
{
    if (!node)
        return;

    int len = strlen(node->name) + 1;

    fwrite(&node->id, 1, sizeof(int), file);
    fwrite(&len, 1, sizeof(int), file);
    fwrite(node->name, 1, len, file);
    fwrite(&node->is_directory, 1, sizeof(char), file);
    if (node->is_directory)
    {
        fwrite(&node->num_children, 1, sizeof(int), file);
        for (int i = 0; i < node->num_children; i++)
        {
            struct inode *child = node->children[i];
            size_t id = child->id;
            fwrite(&id, 1, sizeof(size_t), file);
        }

        for (int i = 0; i < node->num_children; i++)
        {
            struct inode *child = node->children[i];
            save_inode(file, child);
        }
    }
    else
    {
        fwrite(&node->filesize, 1, sizeof(int), file);
        fwrite(&node->num_blocks, 1, sizeof(int), file);
        for (int i = 0; i < node->num_blocks; i++)
        {
            fwrite(&node->blocks[i], 1, sizeof(size_t), file);
        }
    }
}

void save_inodes(char *master_file_table, struct inode *root)
{
    if (root == NULL)
    {
        fprintf(stderr, "root inode is NULL\n");
        return;
    }

    FILE *file = fopen(master_file_table, "w");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s\n", master_file_table);
        return;
    }

    save_inode(file, root);

    fclose(file);
}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

// Do not change.
void debug_fs(struct inode *node)
{
    if (node == NULL)
        fprintf(stderr, "Node is NULL\n");
    return;
    for (int i = 0; i < indent; i++)
        printf("  ");

    if (node->is_directory)
    {
        printf("%s (id %d)\n", node->name, node->id);
        indent++;
        for (int i = 0; i < node->num_children; i++)
        {
            struct inode *child = (struct inode *)node->children[i];
            debug_fs(child);
        }
        indent--;
    }
    else
    {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize);
        for (int i = 0; i < node->num_blocks; i++)
        {
            printf("%d ", (int)node->blocks[i]);
        }
        printf(")\n");
    }
}

// Do not change.
void fs_shutdown(struct inode *inode)
{
    if (!inode)
        return;

    if (inode->is_directory)
    {
        for (int i = 0; i < inode->num_children; i++)
        {
            fs_shutdown(inode->children[i]);
        }
    }

    if (inode->name)
        free(inode->name);
    if (inode->children)
        free(inode->children);
    if (inode->blocks)
        free(inode->blocks);
    free(inode);
}
