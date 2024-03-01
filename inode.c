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
    if (find_inode_by_name(parent, name) != NULL)
    {
        return NULL;
    }

    // creates file
    struct inode file;
    file.id = next_inode_id();
    file.name = name;
    file.is_directory = 0;
    file.filesize = size_in_bytes;
    file.num_blocks = blocks_needed(size_in_bytes);
    size_t *blocks = (size_t *)malloc(file.num_blocks * sizeof(size_t));
    file.blocks = blocks;

    // update parent
    parent->num_children++;
    int last = parent->num_children - 1;
    parent->children[last] = &file;

    return &file;
}

struct inode *create_dir(struct inode *parent, char *name)
{
    if (find_inode_by_name(parent, name) != NULL)
    {
        return NULL;
    }

    struct inode dir;
    dir.id = next_inode_id();
    dir.name = name;
    dir.is_directory = 1;
    dir.num_children = 0;
    dir.children = NULL;

    int num_siblings = parent->num_children++;
    parent->children[(num_siblings) * sizeof(long int)] = &dir;

    return &dir;
}

/* Check all the inodes that are directly referenced by
 * the node parent. If one of them has the name "name",
 * its inode pointer is returned.
 * parent must be directory.
 */
struct inode *find_inode_by_name(struct inode *parent, char *name)
{
    /* Ka - ikke testet
    gå gjennom hvert barn og sjekk navnet deres
    returner peker til barn-inoden hvis funnet */
    if (parent->is_directory == 0)
    {
        return NULL;
    }

    int num_children = parent->num_children;

    for (int i = 0; i < num_children; i++)
    {
        struct inode *child = parent->children[i];
        if (strcmp(child->name, name) == 0)
        {
            return child;
        }
    }

    return NULL;
}

static int verified_delete_in_parent(struct inode *parent, struct inode *node)
{
    // TODO
    (void)parent;
    (void)node;
    return 0;
}

int is_node_in_parent(struct inode *parent, struct inode *node)
{
    // TODO
    (void)parent;
    (void)node;
    return 0;
}

int delete_file(struct inode *parent, struct inode *node)
{
    // TODO
    (void)parent;
    (void)node;
    return 0;
}

int delete_dir(struct inode *parent, struct inode *node)
{
    // TODO
    (void)parent;
    (void)node;
    return 0;
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

    int offset = 0;
    struct inode *root = load_inodes_recursive(file, &offset);

    fclose(file);
    return root;
}

struct inode *load_inodes_recursive(FILE *file, int *offset)
{
    next_inode_id();

    // prep
    struct inode *inode = malloc(sizeof(struct inode));
    fseek(file, *offset, SEEK_SET);

    // ID
    int id;
    fread(&id, sizeof(int), 1, file);
    fseek(file, sizeof(int), SEEK_CUR);
    inode->id = id;

    // name_len;
    int name_len;
    fread(&name_len, sizeof(int), 1, file);
    fseek(file, sizeof(int), SEEK_CUR);

    // name
    char *name_ptr = malloc(name_len);
    fread(name_ptr, 1, name_len, file);
    fseek(file, name_len, SEEK_CUR);
    inode->name = name_ptr;

    // is_directory
    char is_directory;
    fread(&is_directory, sizeof(char), 1, file);
    fseek(file, sizeof(char), SEEK_CUR);
    inode->is_directory = is_directory;

    if (is_directory)
    {
        int num_children;
        fread(&num_children, sizeof(int), 1, file);
        fseek(file, sizeof(int), SEEK_CUR);
        inode->num_children = num_children;

        struct inode **children = malloc(sizeof(struct inode *) * num_children);

        for (int i = 0; i < num_children; i++)
        {
            children[i] = load_inodes_recursive(file, offset);
        }
        inode->children = children;
    }
    else
    {
        int filesize;
        fread(&filesize, sizeof(int), 1, file);
        fseek(file, sizeof(int), SEEK_CUR);
        inode->filesize = filesize;

        int num_blocks;
        fread(&num_blocks, sizeof(int), 1, file);
        fseek(file, sizeof(int), SEEK_CUR);
        inode->num_blocks = num_blocks;

        double *blocks = malloc(sizeof(double) * num_blocks);
        fread(blocks, sizeof(double), num_blocks, file);
        fseek(file, num_blocks * sizeof(double), SEEK_CUR);
        inode->blocks = blocks;
    }

    return inode;
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
