//
// Created by  86136 on 2023/5/23.
// 123

#ifndef LLFS_CONFIG_H
#define LLFS_CONFIG_H

#include <cstdio>
#include <malloc.h>
#include <cstring>
#include "RunningSystem.h"
#include <iostream>
using namespace std;

static char disk_file_name[20] = "D:\\disk.bin";

#define BLOCKSIZ  512   //每个物理块大小
#define SYSOPENFILE 40  //系统打开文件表最大项数
#define DIRNUM  31     //每个目录所包含的最大目录项数（文件数）
#define DIRSIZ  16      //每个目录项名字部分所占字节数，另加i节点号2个字节
#define PWDSIZ   12     //口令字
#define PWDNUM   32     //最多可设32个口令登录
#define NOFILE  20      //每个用户最多可打开20个文件，即用户打开文件最大次数
#define NADDR 10        //每个i节点最多指向10块，addr[0]~addr[9]
#define NHINO  128      //共128个Hash链表，提供索引i节点（必须为2的幂）
#define USERNUM  10     //最多允许10个用户登录
#define DINODESIZ  32   //每个磁盘i节点所占字节
#define DINODEBLK  32   //所有磁盘i节点共占32个物理块
#define FILEBLK  1024    //共有512个目录文件物理块
#define NICFREE  50     //超级块中空闲块数组的最大块数
#define NICINOD  10     //超级块中空闲节点的最大块数
#define DINODESTART (2*BLOCKSIZ)                //i节点起始地址
#define DATASTART ((2+DINODEBLK)*BLOCKSIZ)     //目录、文件区起始地址
enum operation{Open,Read,Write};       //定义操作 打开 读 写

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

/*
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
*/

#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_TRUNC  01000
#define O_APPEND 02000

#define EXICUTE     3
#define DEFAULTMODE 00777
#define IUPDATE     00002
#define SUPDATE     00001
#define FREAD       00001
#define FWRITE      00002
#define FAPPEND     00004
#define DISKFULL    65535
#define SEEK_SET    0


#define READ    1   //只有组内成员（包括创建者）可以读
#define CHANGE  2   //只有创建者可以更改
#define FDELETE  3   //删除
// 写文件方式
#define W_APPEND (-2)      // 追加，即从文件末尾写起，补充原文件
#define W_TRUNC  (-1)      // 截断，即从文件开头写起，原文件作废
// 打开方式
#define BUILD_OPEN      1 //创建打开
#define FP_HEAD_OPEN    2 //在开头打开文件
#define FP_TAIL_OPEN    3 //在末尾打开文件
//openfile返回错误码
#define PERMISSION_DD  -1//权限不足
#define NOT_FOUND      -2//未找到文件
#define DIR_FULL       -3//目录区已满
#define SYS_TABLE_FULL -4//未找到系统打开表空闲表项
#define UER_TABLE_FULL -5//未找到用户打开表空闲表项


#define USER_UNOPENED (-1)      // 当前用户未打开
struct dinode{
    unsigned short di_number;    // 硬连接次数
    unsigned short di_mode;      // 文件类型，目录和文件
    unsigned short di_uid;       // 所有者标识符
    unsigned short di_gid;       // 所在组标识符
    unsigned short di_size;      // 文件的字节数
    unsigned short di_addr[NADDR]; // 文件的硬盘索引数组，即各硬盘节点的id
};

typedef struct inode{
    struct inode *i_forw;
    struct inode *i_back;
    void *      content;
    char ifChange;              //脏位 0未修改/1修改过
    unsigned int d_index;       // 硬盘i节点id
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
    char d_name[DIRSIZ-4];
    unsigned int d_index;            // 硬盘i节点id
};

//目录的逻辑结构,即目录文件的数据区内容
struct dir{
    //files[0]根目录
    //files[1]父目录
    struct FCB files[DIRNUM];
    int size;
};

//用户打开表项
struct user_open_item{
    unsigned int f_count;               //使用进程数
    unsigned short u_default_mode;      //打开方式
    struct inode *f_inode;              //内存i节点指针
    unsigned long f_offset;             //文件偏移量（文件指针）
    short index_to_sysopen;       //系统打开表索引
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


// 路径是否合法
int judge_path(string pathname);

void write_data_back(void *data_address, unsigned short *di_addr, int size, FILE *fp);
#endif //LLFS_CONFIG_H
