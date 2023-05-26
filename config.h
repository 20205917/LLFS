//
// Created by 86136 on 2023/5/23.
//

#ifndef LLFS_CONFIG_H
#define LLFS_CONFIG_H

#include <cstdio>
#include <malloc.h>
#include <cstring>

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
#define NICFREE  50     //超级块中空闲块数组的最大块数  ????????待定
#define NICINOD  50     //超级块中空闲节点的最大块数  ??????待定
#define DINODESTART 2*BLOCKSIZ                //i节点起始地址
#define DATASTART (2+DINODEBLK)*BLOCKSIZ     //目录、文件区起始地址


#define DIEMPTY     00000
#define DIFILE      01000
#define DIDIR       02000
#define UDIREAD     00001
#define UDIWRITE    00002
#define UDIEXICUTE  00004
#define GDIREAD     00010
#define GDIWRITE    00020
#define GDIEXICUTE  00040
#define ODIREAD     00100
#define ODIWRITE    00200
#define ODIEXICUTE  00400
#define READ        1
#define WRITE       2
#define EXICUTE     3
#define DEFAULTMODE 00777
#define IUPDATE     00002
#define SUPDATE     00001
#define FREAD       00001
#define FWRITE      00002
#define FAPPEND     00004
#define DISKFULL    65535
#define SEEK_SET    0

struct dinode{
    unsigned short di_number;    // 硬连接次数
    unsigned short di_mode;      // 文件类型，目录和文件
    unsigned short di_uid;       // 所有者标识符
    unsigned short di_gid;       // 所在组标识符
    unsigned short di_size;      // 文件的字节数
    unsigned int di_addr[NADDR]; // 文件的硬盘索引表，即各硬盘节点的id
};

typedef struct inode{
    struct inode *i_forw;
    struct inode *i_back;
    char i_flag;                //
    char ifChange;              //脏位 0未修改/1修改过
    unsigned int i_id;          // 硬盘i节点id
    struct dinode dinode;
}*hinode;


// 先暂定32块 4块索引 26块数据
struct super_block{
    unsigned long  s_block_size;         //数据块块数

    unsigned int s_dinode_size;          //i节点总数
    unsigned int s_free_dinode_num;      //空闲i节点数
    unsigned int s_dinodes[NICINOD];     //暂时记录的i节点数组
    unsigned int s_pdinode;              //铭记i节点.第一个空的i节点下标
    unsigned int s_rdinode;             //磁盘索引节点搜索记录节点

    unsigned long  s_free_block_size;    //空闲数据块块数
    unsigned int   s_free_blocks[NICFREE]; //空闲块栈,用于成组连接
    unsigned short s_pfree_block;        //空闲块栈栈顶
    char s_fmod;                         //超级块修改标志
};

//口令字
struct PWD{
    unsigned short p_uid;
    unsigned short p_gid;
    char password[PWDSIZ];
};

struct FCB{
    char d_name[DIRSIZ];
    unsigned int d_ino;            // 硬盘i节点id
};

//目录的逻辑结构
struct dir{
    //direct[0]父目录
    //direct[1]当前目录
    struct FCB files[DIRNUM];
    int size;
};

//用户打开表项
struct user_open_item{
    unsigned int f_count;               //使用进程数
    unsigned short u_default_mode;      //打开方式
    struct inode *f_inode;              //内存i节点指针
    unsigned long f_offset;             //文件偏移量（文件指针）
    unsigned short id_to_sysopen;       //系统打开表索引
};

/*
 * 用户打开表
 * 初始化全0
 */
struct user_open_table{
    unsigned short p_uid;
    unsigned short p_gid;
    struct user_open_item items[NOFILE];
};

//系统打开表项
struct sys_open_item{
    FCB fcb;                     // FCB
    unsigned int i_count;        // 打开次数
};


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
extern struct inode *iget(int dinode_id , hinode* hinodes, FILE* disk);
// 释放内存i节点
extern void iput(hinode inode, FILE* disk, struct super_block &file_system);
// 磁盘i节点分配
extern struct dinode * ialloc();
// 磁盘i节点释放
extern void ifree(int dinode_id, struct super_block &file_system);
// 实现对文件的存取搜索，将给定的路径名转换成所要搜索的文件的内存i结点指针（在目录数组中的位置）
// 将会返回在数组中的下标，若为DIRNUM表明没找到
extern unsigned int namei(char* name, hinode cur_path_inode, FILE* disk);
// 在当前目录下搜索到一个空的目录数组，以便建立新的目录或文件时使用
// 将会返回在数组中的下标，若为DIRNUM表明没找到
extern unsigned short iname();
// 磁盘块分配
extern unsigned int balloc(struct super_block &file_system, FILE *disk);
// 磁盘块释放
extern void bfree(int block_num, struct super_block &file_system, FILE* disk);


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
