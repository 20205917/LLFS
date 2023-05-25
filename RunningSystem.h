//
// Created by 86136 on 2023/5/25.
//

#ifndef LLFS_RUNNINGSYSTEM_H
#define LLFS_RUNNINGSYSTEM_H

#include "config.h"

class RunningSystem {


private:
    struct sys_open_item system_openfiles[SYSOPENFILE];  //系统打开表
    struct user_open_item user_openfiles[USERNUM][NOFILE];
    struct dir root;                          //root目录
    hinode hinodes[NHINO];                    //内存节点缓存
    struct super_block file_system;           //超级块
    struct PWD pwds[PWDNUM];                  //用户数组
    FILE *disk;                               //系统磁盘文件
    struct inode *cur_path_inode;             //当前目录

public:
    RunningSystem();
    ~RunningSystem();
    // 打开文件
    unsigned short openFile();
    // 关闭文件
    void closeFile();
    // 读取文件
    unsigned int readFile();
    // 写文件
    unsigned int writeFile();
    // 创建新文件
    bool createFile();
    // 删除文件
    bool deleteFile();
    // 从磁盘文件加载系统
    void install();
    // 格式化系统
    void format();
    // 退出系统
    void halt();

};
unsigned short openFile();
/*

//open file or directory
int ropen(const char *pathname, int flags) {
    //invalid path
    if (justify_path(pathname) == -1) {
        return -1;
    }
    char end = pathname[strlen(pathname) - 1];//get end char
    char *path = clean_path(pathname);
    //find file or directory
    File *file = find_file(path);
    if (file == NULL) {//没找到默认是文件,并且不合法
        if (end == '/') {
            free(path);
            return -1;
        }
        //file or directory not found
        if (flags & O_CREAT) {
            //create file or directory
            file = create_file(path, FILE);
            if (file == NULL) {
                //create file or directory failed
                free(path);
                return -1;
            }
        } else {
            //file or directory not found
            free(path);
            return -1;
        }
    }
    //find the first empty file descriptor
    int fd = 1;
    while (fd_table.fds[fd] != NULL) {
        fd++;
    }
    //create file descriptor
    Fd *fd1 = (Fd *) malloc(sizeof(Fd));

    fd1->flags = flags;
    fd1->file = file;
    fd1->offset = 0;
    //add file descriptor to file descriptor table
    fd_table.fds[fd] = fd1;
    file->link_count++;//link count +1

    if (file->type == FILE) {
        if (flags & O_APPEND) {
            fd1->offset = file->size;
        }
        else {
            fd1->offset = 0;
        }
        //check flags
        if ((flags & O_TRUNC) && ((flags & O_WRONLY) || (flags & O_RDWR))) {
            //truncate file
            file->size = 0;
            free(file->content);
            file->content = NULL;
        }
    }
    free(path);
    return fd;
}

//create file or directory ,choose type FILE or DIRECTORY
File *create_file(const char *pathname, int type) {
    //find parent directory
    char *parent_path = (char *) malloc(strlen(pathname) + 1);
    memset(parent_path, 0, strlen(pathname) + 1);
    strcpy(parent_path, pathname);

    char *name = strrchr(parent_path, '/');
    *name = '\0';
    name++;
    char* tmp= clean_path(parent_path);
    File *parent = find_file(tmp);
    free(tmp);
    if (parent == NULL||parent->type!=DIRECTORY) {
        free(parent_path);
        //parent directory not found
        return NULL;
    }
    //create file or directory
    File *file = (File *) malloc(sizeof(File));
    file->type = type;
    file->size = 0;
    file->parent = parent;
    file->child = NULL;
    file->sibling = NULL;
    file->content = NULL;
    file->link_count=0;
    file->name = (char *) malloc(strlen(name) + 1);
    strcpy(file->name, name);
    //add file or directory to parent directory
    if (parent->child == NULL) {
        parent->child = file;
    } else {
        File *child = parent->child;
        while (child->sibling != NULL) {
            child = child->sibling;
        }
        child->sibling = file;
    }
    free(parent_path);
    return file;
}

//find file
File *find_file(const char *pathname) {
    if (strcmp(pathname, "/")==0|| strcmp(pathname,"") == 0) {
        return root;
    }
    char *tmp = (char *) malloc(strlen(pathname) + 1);
    strcpy(tmp, pathname);
    File *cur = root;//current file
    char *path = strtok(tmp, "/");
    while (path != NULL) {
        if (cur->child == NULL) {
            //child not found
            cur = NULL;
            break;
        }
        cur = cur->child;
        while (cur != NULL) {
            if (strcmp(cur->name, path) == 0) {
                //child found
                break;
            }
            cur = cur->sibling;
        }
        if (cur == NULL) {
            //child not found
            break;
        }
        path = strtok(NULL, "/");
    }
    while (path != NULL) {
        path = strtok(NULL, "/");
    }
    //get the end name of the path and compare with the current file name
    char *name = strrchr(pathname, '/');
    name++;
    if (cur != NULL && strcmp(cur->name, name) != 0) {
        cur = NULL;
    }
    free(tmp);
    return cur;
}

//clear file path
int justify_path(const char *pathname) {
    if (pathname == NULL || strlen(pathname) < 1) {
        return -1;
    }
    if (pathname[0] != '/') {
        return -1;
    }
    //路径长度 <= 1024 字节。（变相地说，文件系统的路径深度存在上限）。
    if (strlen(pathname) > 1024) {
        return -1;
    }
    for (int i = 0; i < strlen(pathname); i++) {
        //only contain letter   number   english point
        if (!((pathname[i] >= 'a' && pathname[i] <= 'z') || (pathname[i] >= 'A' && pathname[i] <= 'Z') ||
              (pathname[i] >= '0' && pathname[i] <= '9') || pathname[i] == '.' || pathname[i] == '/')) {
            return -1;
        }
    }
//    单个文件和目录名长度 <= 32 字节
    char *tmp = (char *) malloc(strlen(pathname) + 1);
    memset(tmp, 0, strlen(pathname) + 1);
    strcpy(tmp, pathname);
    char *path = strtok(tmp, "/");
    while (path != NULL) {
        if (strlen(path) > 32) {
            return -1;
        }
        path = strtok(NULL, "/");
    }
    free(tmp);
    return 0;
}

//create directory
int rmkdir(const char *pathname) {
    int length = strlen(pathname);
    //find . in pathname
    for (int i = 0; i < length; ++i) {
        if (pathname[i] == '.') {
            return -1;
        }
    }
    //invalid path
    if (justify_path(pathname) == -1) {
        return -1;
    }
    //clean path
    char *path = clean_path(pathname);
    //find file first
    File *file = find_file(path);
    if (file != NULL) {
        //file or directory already exists
        free(path);
        return -1;
    }
    //create file or directory
    file = create_file(path, DIRECTORY);
    if (file == NULL) {
        //create file or directory failed
        free(path);
        return -1;
    }
    free(path);
    return 0;
}

//delete directory
int rrmdir(const char *pathname) {
    //find . in pathname
    for (int i = 0; i < strlen(pathname); ++i) {
        if (pathname[i] == '.') {
            return -1;
        }
    }
    //invalid path
    if (strcmp(pathname, "/") == 0) {
        return -1;
    }
    if (justify_path(pathname) == -1) {
        return -1;
    }
    char *path = clean_path(pathname);
    //find file first
    File *file = find_file(path);
    if (file == NULL || file->link_count >= 1||file->type==FILE) {
        //file or directory not found
        free(path);
        return -1;
    }
    if (file->child != NULL) {
        //directory not empty
        free(path);
        return -1;
    }
    //delete file or directory
    File *parent = file->parent;
    if (parent->child == file) {
        parent->child = file->sibling;
    }
    else {
        File *child = parent->child;
        while (child->sibling != file) {
            child = child->sibling;
        }
        child->sibling = file->sibling;
    }
    free(file->name);
    free(file);
    free(path);
    return 0;
}

int runlink(const char *pathname) {
    if (justify_path(pathname) == -1) {
        return -1;
    }
    //find file first
    char *path = clean_path(pathname);

    File *file = find_file(path);
    if (file == NULL) {
        //file or directory not found
        free(path);
        return -1;
    }
    if (file->link_count >= 1||file->type==DIRECTORY) {
        free(path);
        return -1;//link count >=1,can not delete
    }
    //delete file
    File *parent = file->parent;
    if (parent->child == file) {
        parent->child = file->sibling;
    } else {
        File *child = parent->child;
        while (child->sibling != file) {
            child = child->sibling;
        }
        child->sibling = file->sibling;
    }
    free(file->content);
    free(file->name);
    free(file);
    free(path);
    return 0;
}


int rclose(int fd) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL) {
        return -1;
    }
    File *file = fd1->file;
    file->link_count--;//link count -1
    free(fd1);
    fd_table.fds[fd] = NULL;
    return 0;
}


off_t rseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL) {
        return -1;
    }
    File *file = fd1->file;
    if (whence == SEEK_SET) {
        if (offset < 0) {
            return -1;
        }
        fd1->offset = offset;
    } else if (whence == SEEK_CUR) {
        if (offset + fd1->offset < 0) { //offset+fd1->offset < 0
            return -1;
        }
        fd1->offset += offset;
    } else if (whence == SEEK_END) {
        if (offset + fd1->file->size < 0) { //offset+fd1->file->size < 0
            return -1;
        }
        fd1->offset = file->size + offset;
    } else {
        return -1;
    }
    return fd1->offset;
}

ssize_t rread(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL) {
        return -1;
    }
    if (fd1->flags&O_WRONLY) {
        return -1;
    }
    File *file = fd1->file;
    if (file->type == DIRECTORY) {
        return -1;
    }
    //empty file
    if (file->size == 0 || fd1->file->content == NULL) {
        return 0;
    }
    //check the buf
    if (buf == NULL) {
        return -1;
    }
    if (fd1->offset + count > file->size) {
        count = file->size - fd1->offset;
    }
    //check whether the buf size
    memcpy(buf, file->content + fd1->offset, count);
    fd1->offset += (long) count;
    return (long) count;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD_COUNT) {
        return -1;
    }
    Fd *fd1 = fd_table.fds[fd];
    if (fd1 == NULL || (!(fd1->flags & O_WRONLY || fd1->flags & O_RDWR))) {
        return -1;
    }
    File *file = fd1->file;
    if (file == NULL || file->type == DIRECTORY || buf == NULL) {
        return -1;
    }
    if (fd1->offset + count > file->size) {
        if (file->content == NULL) {
            file->content = malloc(fd1->offset + count);
        } else {
            char *tmp = malloc(fd1->offset + count);
            memset(tmp, 0, fd1->offset + count);
            memcpy(tmp, file->content, file->size);
            free(file->content);
            file->content = tmp;
        }
        file->size = (int) fd1->offset + (int) count;//new size
    }
    memcpy(file->content + fd1->offset, buf, count);
    fd1->offset += (long) count;
    return (long) count;
}


char *clean_path(const char *pathname) {
    int length = strlen(pathname);
    char *tmp = (char *) malloc(length + 1);
    memset(tmp, 0, strlen(pathname) + 1);
    int i;
    for (i = length - 1; i >= 0; i--) {
        if (pathname[i] == '/') {
            continue;
        } else {
            break;
        }
    }
    memcpy(tmp, pathname, i + 1);
    tmp[i + 1] = '\0';
    return tmp;
}
*/
#endif //LLFS_RUNNINGSYSTEM_H
