//
// Created by 86136 on 2023/5/25.
//

#ifndef LLFS_RUNNINGSYSTEM_H
#define LLFS_RUNNINGSYSTEM_H

#include <map>
#include "config.h"

using namespace std;

extern struct sys_open_item system_openfiles[SYSOPENFILE];  //系统打开表
extern map<string, user_open_table*> user_openfiles;        //用户打开表组
extern struct dir root;                          //root目录
extern hinode hinodes[NHINO];                    //内存节点缓存
extern struct super_block file_system;           //超级块
extern struct PWD pwds[PWDNUM];                  //用户数组

extern struct inode *cur_dir_inode;             //当前目录的索引结点
extern struct dir cur_dir;                       //当前目录的数据
extern string cur_user;                          //当前用户
extern FILE *disk;                               //系统磁盘文件


    // 打开文件
    int openFile(const string& pathname,unsigned short flags);
    // 关闭文件
    void closeFile(const string& pathname);
    // 读取文件
    string readFile(string pathname);
    /* 写文件
     write_mode可为W_APPEND W_TRUNC 或其他任意值
     W_APPEND追加写 W_TRUNC重置 任意值表示从指定位置写
     返回值false写失败 true写成功
     */
    bool writeFile(const string& pathname, int write_mode, const string& content);
    // 创建新文件
    inode* createFile(string pathname, unsigned short di_mode);
    // 删除文件
    bool deleteFile(string pathname);
    // 从磁盘文件加载系统
    void install();
    // 格式化系统
    void format();
    // 退出系统
    void halt();

    //根据地址寻找并读入索引节点
    struct inode* find_file(string addr);

    // 用户登录 -1.口令错误 -2.已经登录-3.已经达登录上限 >0.登录成功(返回值为用户打开表下标)
    int login(const string& pwd);
    // 用户注销
    void logout(const string& pwd);
    //判断用户权限是否足够某操作
    bool access(int operation, inode *file_inode);
    // 返回当前用户ss
    string whoami();

    // 文件夹路径相关
    int mkdir(string& pathname);     //创建文件夹
    int chdir(const string& pathname);     //更改系统的当前文件路径
    int show_dir();                 //展示当前文件路径的内容
    int rmdir(const string& pathname);     //删除该路径下的文件夹
    struct dir get_dir(unsigned int d_index);//根据d_index，获取dir

    // 判断是否被当前用户打开,若打开返回用户打开表下表,未打开返回USER_UNOPENED
    bool isOpened(string pathname);

    int seek_catalog_leisure(inode *catalog);
    // 磁盘i节点分配
    int ialloc(unsigned int);

    void ifree(unsigned int dinode_id);

    inode* iget(unsigned int dinode_id);

    bool iput(inode* inode);

    void bfree(int block_num);

    unsigned int balloc();

    unsigned int namei(string name);

    void file_wirte_back(struct inode* inode);

#endif //LLFS_RUNNINGSYSTEM_H