#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "node.h"

void appendNode(linkedList *L, node *current) {
    node *head = L->head;

    if (L->head == NULL) {
        L->head = current;
    } else if (strcmp(current->file_name, head->file_name) < 0) {
        L->head = current;
        current->next = head;
    } else {
        while (head->next != NULL &&
               strcmp(current->file_name, head->file_name) > 0) {
            head = head->next;
        }

        current->next = head->next;
        head->next = current;
    }
}

void createNode(node *newNode, struct dirent *dirInfo) {
    struct stat fileInfo;

    stat(dirInfo->d_name, &fileInfo);

    const struct passwd *userInfo = getpwuid(fileInfo.st_uid);

    //  initalize data
    newNode->file_name = dirInfo->d_name;
    newNode->owner_name = userInfo->pw_name;
    newNode->size = fileInfo.st_size / 1024.0;
    newNode->file_perm = fileInfo.st_mode;
    newNode->device = fileInfo.st_dev;
    newNode->inode = dirInfo->d_ino;

    //  initialize link
    newNode->next = NULL;
}

void printPerm(node *node) {
    //  directory
    if (S_ISDIR(node->file_perm)) {
        printf("d");
    } else {
        printf("-");
    }

    //  user
    if (S_IRUSR & node->file_perm) {
        printf("r");
    } else {
        printf("-");
    }

    if (S_IWUSR & node->file_perm) {
        printf("w");
    } else {
        printf("-");
    }

    if (S_IXUSR & node->file_perm) {
        printf("x");
    } else {
        printf("-");
    }

    //  group
    if (S_IRGRP & node->file_perm) {
        printf("r");
    } else {
        printf("-");
    }

    if (S_IWGRP & node->file_perm) {
        printf("w");
    } else {
        printf("-");
    }

    if (S_IXGRP & node->file_perm) {
        printf("x");
    } else {
        printf("-");
    }

    //  others
    if (S_IROTH & node->file_perm) {
        printf("r");
    } else {
        printf("-");
    }

    if (S_IWOTH & node->file_perm) {
        printf("w");
    } else {
        printf("-");
    }

    if (S_IXOTH & node->file_perm) {
        printf("x");
    } else {
        printf("-");
    }
}
