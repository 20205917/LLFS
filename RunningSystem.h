//
// Created by 86136 on 2023/5/25.
//

#ifndef LLFS_RUNNINGSYSTEM_H
#define LLFS_RUNNINGSYSTEM_H

#include <map>
#include "config.h"

using namespace std;

extern struct sys_open_item system_openfiles[SYSOPENFILE];  //ϵͳ�򿪱�
extern map<string, user_open_table*> user_openfiles;        //�û��򿪱���
extern struct dir root;                          //rootĿ¼
extern hinode hinodes[NHINO];                    //�ڴ�ڵ㻺��
extern struct super_block file_system;           //������
extern struct PWD pwds[PWDNUM];                  //�û�����

extern struct inode *cur_dir_inode;             //��ǰĿ¼���������
extern struct dir cur_dir;                       //��ǰĿ¼������
extern string cur_user;                          //��ǰ�û�
extern FILE *disk;                               //ϵͳ�����ļ�


    // ���ļ�
    int openFile(const string& pathname,unsigned short flags);
    // �ر��ļ�
    void closeFile(const string& pathname);
    // ��ȡ�ļ�
    string readFile(string pathname);
    /* д�ļ�
     write_mode��ΪW_APPEND W_TRUNC ����������ֵ
     W_APPEND׷��д W_TRUNC���� ����ֵ��ʾ��ָ��λ��д
     ����ֵfalseдʧ�� trueд�ɹ�
     */
    bool writeFile(const string& pathname, int write_mode, const string& content);
    // �������ļ�
    inode* createFile(string pathname, unsigned short di_mode);
    // ɾ���ļ�
    bool deleteFile(string pathname);
    // �Ӵ����ļ�����ϵͳ
    void install();
    // ��ʽ��ϵͳ
    void format();
    // �˳�ϵͳ
    void halt();

    //���ݵ�ַѰ�Ҳ����������ڵ�
    struct inode* find_file(string addr);

    // �û���¼ -1.������� -2.�Ѿ���¼-3.�Ѿ����¼���� >0.��¼�ɹ�(����ֵΪ�û��򿪱��±�)
    int login(const string& pwd);
    // �û�ע��
    void logout(const string& pwd);
    //�ж��û�Ȩ���Ƿ��㹻ĳ����
    bool access(int operation, inode *file_inode);
    // ���ص�ǰ�û�ss
    string whoami();

    // �ļ���·�����
    int mkdir(string& pathname);     //�����ļ���
    int chdir(const string& pathname);     //����ϵͳ�ĵ�ǰ�ļ�·��
    int show_dir();                 //չʾ��ǰ�ļ�·��������
    int rmdir(const string& pathname);     //ɾ����·���µ��ļ���
    struct dir get_dir(unsigned int d_index);//����d_index����ȡdir

    // �ж��Ƿ񱻵�ǰ�û���,���򿪷����û��򿪱��±�,δ�򿪷���USER_UNOPENED
    bool isOpened(string pathname);

    int seek_catalog_leisure(inode *catalog);
    // ����i�ڵ����
    int ialloc(unsigned int);

    void ifree(unsigned int dinode_id);

    inode* iget(unsigned int dinode_id);

    bool iput(inode* inode);

    void bfree(int block_num);

    unsigned int balloc();

    unsigned int namei(string name);

    void file_wirte_back(struct inode* inode);

#endif //LLFS_RUNNINGSYSTEM_H