//
// Created by 86136 on 2023/5/23.
// 123

#ifndef LLFS_CONFIG_H
#define LLFS_CONFIG_H

#include <cstdio>
#include <malloc.h>
#include <cstring>

#define BLOCKSIZ  512   //ÿ���С
#define SYSOPENFILE 40  //ϵͳ���ļ����������
#define DIRNUM  128     //ÿ��Ŀ¼�����������Ŀ¼�������ļ�����
#define DIRSIZ  14      //ÿ��Ŀ¼�����ֲ�����ռ�ֽ���������i�ڵ��2���ֽ�
#define PWDSIZ   12     //������
#define PWDNUM   32     //������32�������¼
#define NOFILE  20      //ÿ���û����ɴ�20���ļ������û����ļ�������
#define NADDR 10        //ÿ��i�ڵ����ָ��10�飬addr[0]~addr[9]
#define NHINO  128      //��128��Hash�������ṩ����i�ڵ㣨����Ϊ2���ݣ�
#define USERNUM  10     //�������10���û���¼
#define DINODESIZ  32   //ÿ������i�ڵ���ռ�ֽ�
#define DINODEBLK  32   //���д���i�ڵ㹲ռ32��������
#define FILEBLK  512    //����512��Ŀ¼�ļ�������
#define NICFREE  50     //�������п��п������������  ????????����
#define NICINOD  50     //�������п��нڵ��������  ??????����
#define DINODESTART 2*BLOCKSIZ                //i�ڵ���ʼ��ַ
#define DATASTART (2+DINODEBLK)*BLOCKSIZ     //Ŀ¼���ļ�����ʼ��ַ


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
    unsigned short di_number;    // Ӳ���Ӵ���
    unsigned short di_mode;      // �ļ����ͣ�Ŀ¼���ļ�
    unsigned short di_uid;       // �����߱�ʶ��
    unsigned short di_gid;       // �������ʶ��
    unsigned short di_size;      // �ļ����ֽ���
    unsigned int di_addr[NADDR]; // �ļ���Ӳ��������
};

typedef struct inode{
    struct inode *i_forw;
    struct inode *i_back;
    char i_flag;                //
    char ifChange;              //��λ 0δ�޸�/1�޸Ĺ�
    unsigned int i_id;          // Ӳ��i�ڵ�id
    struct dinode dinode;
}*hinode;


// ���ݶ�32�� 4������ 26������
struct super_block{
    unsigned long  s_block_size;         //���ݿ����

    unsigned int s_dinode_size;          //i�ڵ�����
    unsigned int s_free_dinode_num;      //����i�ڵ���
    unsigned int s_dinodes[NICINOD];     //��ʱ��¼��i�ڵ�����
    unsigned int s_pdinode;              //����i�ڵ�.��һ���յ�i�ڵ��±�
    unsigned int s_rdinode;             //���������ڵ�������¼�ڵ�

    unsigned long  s_free_block_size;    //�������ݿ����
    unsigned int   s_free_blocks[NICFREE]; //���п�ջ,���ڳ�������
    unsigned short s_pfree_block;        //���п�ջջ��
    char s_fmod;                         //�������޸ı�־
};

//������
struct PWD{
    unsigned short p_uid;
    unsigned short p_gid;
    char password[PWDSIZ];
};

struct FCB{
    char d_name[DIRSIZ];
    unsigned int d_ino;            // Ӳ��i�ڵ�id
};

//Ŀ¼���߼��ṹ
struct dir{
    //direct[0]��Ŀ¼
    //direct[1]��ǰĿ¼
    struct FCB files[DIRNUM];
    int size;
};

//�û��򿪱���
struct user_open_item{
    unsigned int f_count;               //ʹ�ý�����
    unsigned short u_default_mode;      //�򿪷�ʽ
    struct inode *f_inode;              //�ڴ�i�ڵ�ָ��
    unsigned long f_offset;             //�ļ�ƫ�������ļ�ָ�룩
    unsigned short id_to_sysopen;       //ϵͳ�򿪱�����
};

/*
 * �û��򿪱�
 * ��ʼ��ȫ0
 */
struct user_open_table{
    unsigned short p_uid;
    unsigned short p_gid;
    struct user_open_item items[NOFILE];
};

//ϵͳ�򿪱���
struct sys_open_item{
    FCB fcb;                     // FCB
    unsigned int i_count;        // �򿪴���
};


// ���ļ�
extern unsigned short openFile();
// �ر��ļ�
extern void closeFile();
// ��ȡ�ļ�
extern unsigned int readFile();
// д�ļ�
extern unsigned int writeFile();
// �������ļ�
extern bool createFile();
// ɾ���ļ�
extern bool deleteFile();

// ������Ŀ¼
extern bool mkdir();
// �ı䵱ǰ����Ŀ¼
extern bool chdir();
// չʾ��ǰĿ¼
extern void show_dir();

extern bool hard_link(); //Ӳ����
extern bool soft_link(); //������

// ��ȡ�ڴ�i�ڵ�
extern struct inode *iget(int dinode_id , hinode* &hinodes, FILE* disk);
// �ͷ��ڴ�i�ڵ�
extern void iput(hinode inode, FILE* disk, struct super_block &file_system);
// ����i�ڵ����
extern struct dinode * ialloc();
// ����i�ڵ��ͷ�
extern void ifree(int dinode_id, struct super_block &file_system);
// ʵ�ֶ��ļ��Ĵ�ȡ��������������·����ת������Ҫ�������ļ����ڴ�i���ָ�루��Ŀ¼�����е�λ�ã�
extern unsigned int namei();
// �ڵ�ǰĿ¼��������һ���յ�Ŀ¼���飬�Ա㽨���µ�Ŀ¼���ļ�ʱʹ��
extern unsigned short iname();
// ���̿����
extern unsigned int balloc();
// ���̿��ͷ�
extern void bfree(int block_num, struct super_block &file_system, FILE* disk);


// ����
// ɾ��ָ��Ŀ¼
extern void rmdir();
// ����
extern void copy();
// �ı��ļ�������
extern bool chown();
// �ı��ļ�������
extern bool chgrp();
// �ı��û�������
extern bool usermod();
// �ı�Ȩ��
extern bool chmod();
// ������
extern bool groupadd();
// �鿴������
extern void show_group();
// �鿴�����û�
extern void show_users();
// �鿴��ǰ�û�
extern void shoami();
#endif //LLFS_CONFIG_H
