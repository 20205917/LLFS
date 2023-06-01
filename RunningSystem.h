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

extern hinode hinodes[NHINO];                    //内存节点缓存
extern struct super_block file_system;           //超级块
extern struct PWD pwds[PWDNUM];                  //用户数组

extern struct inode *cur_dir_inode;             //当前目录的索引结点
extern string cur_user;                          //当前用户
extern string cur_path;                        //当前目录名
// 现在分配有2+DINODEBLK+FILEBLK个磁盘块
// 前2个为引导快（现在放了用户信息）、超级块
// 索引区有DINODEBLK个块，数据区FILEBLK个块
extern FILE *disk;                               //系统磁盘文件

    int hard_link(string &pathname,string &newname);
    // 打开文件
    int openFile(const string& pathname, unsigned short flags);
    int open_file(string& pathname, int operation);
    // 关闭文件
    void closeFile(const string& pathname);
    int close_file(int fd);
    // 读取文件
    string readFile(int fd);
    /* 写文件
     write_mode可为W_APPEND W_TRUNC 或其他任意值
     W_APPEND追加写 W_TRUNC重置 任意值表示从指定位置写
     返回值false写失败 true写成功
     */
    bool writeFile(const string& pathname, int write_mode, const string& content);
    bool writeFile(int fd, const string& content);
    // 创建新文件
    int createFile(string pathname);
    // 删除文件
    int deleteFile(string pathname);
    // 初始化
    void initial();
    // 从磁盘文件加载系统
    void install();
    // 格式化系统
    void format();
    // 退出系统
    void halt();

    //根据地址寻找并读入索引节点
    struct inode* find_file(string addr);
    // 申请磁盘i节点
    struct inode *ialloc();
    // 用户登录 -1.口令错误 -2.已经登录-3.已经达登录上限 >0.登录成功(返回值为用户打开表下标)
    int login(const string& pwd);
    // 用户注销
    void logout(const string& pwd);
    //判断用户权限是否足够某操作
    bool access(int operation, inode *file_inode);
    // 返回当前用户
    string whoami();
    void show_all_users();
    void show_login_users();
    int switch_user(const string& pwd);
    int usermod(int uid, int gid);
    int useradd(int gid, const std::string& pwd);

    // 文件夹路径相关
    int mkdir(string& pathname);     //创建文件夹
    int chdir(string& pathname);     //更改系统的当前文件路径
    int show_dir();                 //展示当前文件路径的内容
    int show_whole_dir();           // 展示文件系统整个目录结构
    int show_dir_tree(unsigned int id, int depth);
    int rmdir(string& pathname);     //删除该路径下的文件夹
    struct dir get_dir(unsigned int d_index);//根据d_index，获取dir

    //文件所属相关
    int change_file_owner(string& pathname, int uid);   //改变文件所属用户
    int change_file_group(string& pathname, int gid);   //改变文件所在组

    // 判断是否被当前用户打开,若打开返回用户打开表下表,未打开返回USER_UNOPENED
    bool isOpened(string pathname);

    int seek_catalog_leisure();
    // 磁盘i节点分配
    int ialloc(unsigned int);

    void ifree(unsigned int dinode_id);

    inode* iget(unsigned int dinode_id);

    bool iput(inode* inode);

    void bfree(int block_num);

    unsigned int balloc();

    unsigned int namei(string name);

    void file_wirte_back(struct inode* inode);

    inode* getDinodeFromDisk(int dinode_id);

//查看某个磁盘i节点id对应的内存i节点是否存在
    inode* findHinode(int dinode_id);

#endif //LLFS_RUNNINGSYSTEM_H