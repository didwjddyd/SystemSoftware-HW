#ifndef INCLUDED_NODE
#define INCLUDED_NODE

#include <dirent.h>
#include <sys/stat.h>

typedef struct __linkedList {
    struct __node *head;
    // struct __node *tail;
} linkedList;

typedef struct __node {
    //  data
    const char *file_name;  //  file name
    const char *owner_name; //  owner name

    float size; //  file size

    mode_t file_perm; //  file permission

    dev_t device; //  device number

    ino_t inode; //  inode number

    //  link
    struct __node *next;
} node;

typedef struct __counter {
    int dirs;
    int files;
} counter;

void appendNode(linkedList *, node *);
void createNode(node *, struct dirent *);
void printPerm(node *);

#endif
