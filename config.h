//
// Created by 86136 on 2023/5/23.
//

#ifndef LLFS_CONFIG_H
#define LLFS_CONFIG_H

#include <cstdio>

#define BLOCKSIZ  512   //每块大小
#define SYSOPENFILE 40  //系统打开文件表最大项数
#define DIRNUM  128     //每个目录所包含的最大目录项数（文件数）
#define DIRSIZ  14      //每个目录项名字部分所占字节数，另加i节点号2个字节
#define PWDSIZ   12     //口令字
#define PWDNUM   32     //最多可设32个口令登录
#define NOFILE  20      //每个用户最多可打开20个文件，即用户打开文件最大次数
#define NADDR 10        //每个i节点最多指向10块，addr[0]~addr[9]
#define NHINO  128      //共128个Hash链表，提供索引i节点（必须为2的幂）
#define USERNUM  10     //最多允许10个用户登录
#define DINODESIZ  32   //每个磁盘i节点所占字节
#define DINODEBLK  32   //所有磁盘i节点共占32个物理块
#define FILEBLK  512    //共有512个目录文件物理块
#define NICFREE  50     //超级块中空闲块数组的最大块数
#define NICINOD  50     //超级块中空闲节点的最大块数
#define DINODESTART 2*BLOCKSIZ                //i节点起始地址
#define DATASTART (2+DINODEBLK)*BLOCKSIZ     //目录、文件区起始地址

struct dinode{
    unsigned short di_number;    // 硬连接次数
    unsigned short di_mode;      // 文件类型，目录和文件
    unsigned short di_uid;       // 所有者标识符
    unsigned short di_gid;       // 所在组标识符
    unsigned short di_size;      // 文件的字节数
    unsigned int di_addr[NADDR]; // 文件的硬盘索引表
};

typedef struct inode{
    struct inode *i_forw;
    struct inode *i_back;
    char i_flag;
    unsigned int i_id;           // 硬盘i节点id
    unsigned int i_count;        // 打开次数
    struct dinode dinode;
}*hinode;



struct direct{
    char d_name[DIRSIZ];
    unsigned int d_ino;            // 硬盘i节点id
};

struct dir{
    //direct[0]父目录
    //direct[1]当前目录
    struct direct direct[DIRNUM];
    int size;
};

//口令字
struct PWD{
    unsigned short p_uid;
    unsigned short p_gid;
    char password[PWDSIZ];
};

//文件标识符
struct fd{
    char f_flag;            //打开方式
    unsigned int f_count;   //使用进程数
    struct inode *f_inode;  //内存i节点指针
    unsigned long f_offset;    //文件偏移量（文件指针）
};

struct super_block{
    unsigned short s_dinode_size;        //磁盘索引节点表总块数
    unsigned long  s_block_size;         //数据块块数

    unsigned int s_free_dinode_num;      //空闲i节点数
    unsigned int s_dinodes[NICINOD];     //空闲i节点数组
    unsigned int s_rdinode;              //铭记i节点，由它决定从索引节点区搜索空闲i节点的起始位置。

    unsigned long  s_free_block_size;    //空闲数据块块数
    unsigned int s_free_blocks[NICFREE]; //空闲块栈,用于成组连接
    unsigned short s_pfree_block;        //空闲块栈栈顶
    char s_fmod;                         //超级块修改标志
};


//用户打开表
struct user{
    unsigned short u_default_mode;
    unsigned short u_uid;           // 用户标识符
    unsigned short u_gid;           // 所在组标识符
    unsigned short u_ofile[NOFILE]; //用户打开表
};

extern hinode hinodes[NHINO];
extern struct dir root;                          //root目录
extern struct fd system_openfiles[SYSOPENFILE];  //系统打开表
extern struct super_block file_system;           //超级块
extern struct PWD pwds[PWDNUM];                  //用户数组
extern struct user user[USERNUM];                //登录用户
extern FILE *disk;                             //系统磁盘文件
extern struct inode *cur_path_inode;             //当前目录


// 打开文件
extern unsigned short openFile();
// 关闭文件
extern void closeFile();
// 读取文件
extern unsigned int readFile();
// 写文件
extern unsigned int writeFile();
// 创建新文件
extern bool createFile();
// 删除文件
extern bool deleteFile();

// 创建新目录
extern bool mkdir();
// 改变当前所在目录
extern bool chdir();
// 展示当前目录
extern void show_dir();

extern bool hard_link(); //硬链接
extern bool soft_link(); //软连接

// 获取内存i节点
extern struct inode *iget();
// 释放内存i节点
extern void iput();
// 磁盘i节点分配
extern struct inode * ialloc();
// 磁盘i节点释放
extern void ifree();
// 实现对文件的存取搜索，将给定的路径名转换成所要搜索的文件的内存i结点指针（在目录数组中的位置）
extern unsigned int namei();
// 在当前目录下搜索到一个空的目录数组，以便建立新的目录或文件时使用
extern unsigned short iname();
// 磁盘块分配
extern unsigned int balloc();
// 磁盘块释放
extern void bfree();

// 确定是否有权限
extern unsigned int access();
// 用户登录
extern int login();
// 用户注销
extern void logout();

// 从磁盘文件加载系统
extern void install();
// 格式化系统
extern void format();
// 退出系统
extern void halt();

// 额外
// 删除指定目录
extern void rmdir();
// 复制
extern void copy();
// 改变文件所有者
extern bool chown();
// 改变文件所在组
extern bool chgrp();
// 改变用户所在组
extern bool usermod();
// 改变权限
extern bool chmod();
// 添加组
extern bool groupadd();
// 查看所有组
extern void show_group();
// 查看所有用户
extern void show_users();
// 查看当前用户
extern void shoami();
#endif //LLFS_CONFIG_H
