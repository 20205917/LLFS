//
// Created by 86136 on 2023/5/23.
//

#ifndef LLFS_CONFIG_H
#define LLFS_CONFIG_H

#include <cstdio>

#define BLOCKSIZ  512   //ÿ���С
#define SYSOPENFILE 40  //ϵͳ���ļ����������
#define DIRNUM  128     //ÿ��Ŀ¼�����������Ŀ¼�������ļ�����
#define DIRSIZ  14      //ÿ��Ŀ¼�����ֲ�����ռ�ֽ��������i�ڵ��2���ֽ�
#define PWDSIZ   12     //������
#define PWDNUM   32     //������32�������¼
#define NOFILE  20      //ÿ���û����ɴ�20���ļ������û����ļ�������
#define NADDR 10        //ÿ��i�ڵ����ָ��10�飬addr[0]~addr[9]
#define NHINO  128      //��128��Hash�����ṩ����i�ڵ㣨����Ϊ2���ݣ�
#define USERNUM  10     //�������10���û���¼
#define DINODESIZ  32   //ÿ������i�ڵ���ռ�ֽ�
#define DINODEBLK  32   //���д���i�ڵ㹲ռ32�������
#define FILEBLK  512    //����512��Ŀ¼�ļ������
#define NICFREE  50     //�������п��п������������
#define NICINOD  50     //�������п��нڵ��������
#define DINODESTART 2*BLOCKSIZ                //i�ڵ���ʼ��ַ
#define DATASTART (2+DINODEBLK)*BLOCKSIZ     //Ŀ¼���ļ�����ʼ��ַ

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
    char i_flag;
    unsigned int i_id;           // Ӳ��i�ڵ�id
    unsigned int i_count;        // �򿪴���
    struct dinode dinode;
}*hinode;



struct direct{
    char d_name[DIRSIZ];
    unsigned int d_ino;            // Ӳ��i�ڵ�id
};

struct dir{
    //direct[0]��Ŀ¼
    //direct[1]��ǰĿ¼
    struct direct direct[DIRNUM];
    int size;
};

//������
struct PWD{
    unsigned short p_uid;
    unsigned short p_gid;
    char password[PWDSIZ];
};

//�ļ���ʶ��
struct fd{
    char f_flag;            //�򿪷�ʽ
    unsigned int f_count;   //ʹ�ý�����
    struct inode *f_inode;  //�ڴ�i�ڵ�ָ��
    unsigned long f_offset;    //�ļ�ƫ�������ļ�ָ�룩
};

struct super_block{
    unsigned short s_dinode_size;        //���������ڵ���ܿ���
    unsigned long  s_block_size;         //���ݿ����

    unsigned int s_free_dinode_num;      //����i�ڵ���
    unsigned int s_dinodes[NICINOD];     //����i�ڵ�����
    unsigned int s_rdinode;              //����i�ڵ㣬���������������ڵ�����������i�ڵ����ʼλ�á�

    unsigned long  s_free_block_size;    //�������ݿ����
    unsigned int s_free_blocks[NICFREE]; //���п�ջ,���ڳ�������
    unsigned short s_pfree_block;        //���п�ջջ��
    char s_fmod;                         //�������޸ı�־
};


//�û��򿪱�
struct user{
    unsigned short u_default_mode;
    unsigned short u_uid;           // �û���ʶ��
    unsigned short u_gid;           // �������ʶ��
    unsigned short u_ofile[NOFILE]; //�û��򿪱�
};

extern hinode hinodes[NHINO];
extern struct dir root;                          //rootĿ¼
extern struct fd system_openfiles[SYSOPENFILE];  //ϵͳ�򿪱�
extern struct super_block file_system;           //������
extern struct PWD pwds[PWDNUM];                  //�û�����
extern struct user user[USERNUM];                //��¼�û�
extern FILE *disk;                             //ϵͳ�����ļ�
extern struct inode *cur_path_inode;             //��ǰĿ¼


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
extern struct inode *iget();
// �ͷ��ڴ�i�ڵ�
extern void iput();
// ����i�ڵ����
extern struct inode * ialloc();
// ����i�ڵ��ͷ�
extern void ifree();
// ʵ�ֶ��ļ��Ĵ�ȡ��������������·����ת������Ҫ�������ļ����ڴ�i���ָ�루��Ŀ¼�����е�λ�ã�
extern unsigned int namei();
// �ڵ�ǰĿ¼��������һ���յ�Ŀ¼���飬�Ա㽨���µ�Ŀ¼���ļ�ʱʹ��
extern unsigned short iname();
// ���̿����
extern unsigned int balloc();
// ���̿��ͷ�
extern void bfree();

// ȷ���Ƿ���Ȩ��
extern unsigned int access();
// �û���¼
extern int login();
// �û�ע��
extern void logout();

// �Ӵ����ļ�����ϵͳ
extern void install();
// ��ʽ��ϵͳ
extern void format();
// �˳�ϵͳ
extern void halt();

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
// �����
extern bool groupadd();
// �鿴������
extern void show_group();
// �鿴�����û�
extern void show_users();
// �鿴��ǰ�û�
extern void shoami();
#endif //LLFS_CONFIG_H
