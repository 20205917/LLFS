//
// Created by 86136 on 2023/5/25.
//

#ifndef LLFS_RUNNINGSYSTEM_H
#define LLFS_RUNNINGSYSTEM_H

#include <map>
#include "config.h"

using namespace std;
struct RunningSystem {

    struct sys_open_item system_openfiles[SYSOPENFILE];  //系统打开表
    map<string ,user_open_table*> user_openfiles;        //用户打开表组
    struct dir root;                          //root目录
    hinode hinodes[NHINO];                    //内存节点缓存
    struct super_block file_system;           //超级块
    struct PWD pwds[PWDNUM];                  //用户数组

    struct inode *cur_path_inode;             //当前目录
    string cur_user;                          //当前用户
    FILE *disk;                               //系统磁盘文件
    RunningSystem();
    ~RunningSystem();
    // 打开文件
    int openFile(const char *pathname, int flags);
    // 关闭文件
    void closeFile();
    // 读取文件
    unsigned int readFile();
    // 写文件
    unsigned int writeFile();
    // 创建新文件
    inode* createFile(const char *pathname, unsigned short di_mode);
    // 删除文件
    bool deleteFile();
    // 从磁盘文件加载系统
    void install();
    // 格式化系统
    void format();
    // 退出系统
    void halt();

    //根据地址寻找并读入索引节点
    struct inode* find_file(char* addr);


    // 用户登录 -1.口令错误 -2.已经登录-3.已经达登录上限 >0.登录成功(返回值位用户打开表下标)
    int login(string pwd);
    // 用户注销
    void logout(string pwd);

    bool access(string PWD,inode* file_inode);
};



#endif //LLFS_RUNNINGSYSTEM_H
