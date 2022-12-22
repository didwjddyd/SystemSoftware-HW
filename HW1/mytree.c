#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "node.h"

#define MAX_PATH_LEN 4096

void errorMsg(const char *);
void myTree(const char *, char *, counter *);

int main(int argc, char const *argv[]) {
    struct stat fileInfo;

    counter c = {0, 0};

    char cwd[MAX_PATH_LEN + 1] = {
        '\0',
    };

    if (argc != 2) {
        if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
            errorMsg("getcwd() error");
        }

        if (stat(cwd, &fileInfo) == -1) {
            errorMsg("stat() error");
        }

        if (!S_ISDIR(fileInfo.st_mode)) {
            errorMsg("error opening dir");
        }

        printf(".\n");

        myTree(cwd, "", &c);
    } else {
        if (stat(argv[1], &fileInfo) == -1) {
            errorMsg("stat() error");
        }

        if (!S_ISDIR(fileInfo.st_mode)) {
            errorMsg("error opening dir");
        }

        printf("%s\n", argv[1]);

        chdir(argv[1]);

        getcwd(cwd, MAX_PATH_LEN);

        myTree(cwd, "", &c);
    }

    return 0;
}

void errorMsg(const char *msg) {
    perror(msg);
    exit(-1);
}

void myTree(const char *pathname, char *prefix, counter *c) {
    linkedList *L = (linkedList *)malloc(sizeof(linkedList));
    L->head = NULL;

    node *current;

    struct dirent *dirInfo;
    DIR *dirp;

    char *newPrefix, *pointer, *blank;
    char cwd[MAX_PATH_LEN + 1] = {
        '\0',
    };

    dirp = opendir(pathname);

    while ((dirInfo = readdir(dirp)) != NULL) {
        if ((dirInfo->d_name)[0] != '.') {
            current = (node *)malloc(sizeof(node));
            createNode(current, dirInfo);
            appendNode(L, current);
        }
    }

    // print
    node *p = L->head;

    while (p != NULL) {

        if (p->next == NULL) {
            pointer = "└──";
            blank = "    ";
        } else {
            pointer = "├──";
            blank = "│   ";
        }

        printf("%s%s", prefix, pointer);

        //  format: [inode device permission owner size]  name
        printf(" [%ld %ld ", p->inode, p->device);

        printPerm(p);

        printf(" %s %10.1fK]  %s\n", p->owner_name, p->size, p->file_name);

        if (S_ISDIR(p->file_perm)) {
            char *newPrefix = (char *)calloc(strlen(prefix) + strlen(blank) + 1,
                                             sizeof(char));

            strcat(newPrefix, prefix);
            strcat(newPrefix, blank);

            if (chdir(p->file_name) == -1) {
                errorMsg("myTree: chdir() error");
            }

            if (getcwd(cwd, MAX_PATH_LEN) == NULL) {
                errorMsg("myTree: getcwd() error");
            }
            myTree(cwd, newPrefix, c);

            free(newPrefix);
        }

        c->files++;
        p = p->next;
    }

    free(L);

    closedir(dirp);
    chdir("..");
}
