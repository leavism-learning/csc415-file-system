#include "mfs.h"
#include "bfs.h"
#include "fsLow.h"

int fs_mkdir(const char *pathname, mode_t mode)
{
    struct bfs_dir_entry *parent;
    struct bfs_dir_entry dentry;
    char * lastElement;
    //the whole path until the new dir to make
    char * parentPath = "";

    //for relative path
    if(pathname[0] != '/')
    {
        parent = bfs_cwd;
        lastElement = pathname;
    }
    else            //for absolute path
    {
        char * token1 = strtok(pathname, '/');
        char *token2;

        while(token1 != NULL)
        {
            token2 = strtok(NULL, '/');
            if(token2 == NULL)
            {
                lastElement = token1;
                break;
            }
            strcat(parentPath, '/');
            strcat(parentPath, token1);
            token1 = token2;
        }

        if(get_file_from_filepath(parent, parentPath) != 0)
        {
            printf(stderr, "Incorrect parent path!");
        }

    }

    if(create_dir_entry(dentry, lastElement, sizeof(struct bfs_dir_entry), 0) != 0)
    {
        printf(stderr, "Name provided is too long!");
    }

    //TODO: have to finish and use bfs_create_directory



}