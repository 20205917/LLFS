//
// Created by 86136 on 2023/5/25.
//

#ifndef LLFS_RUNNINGSYSTEM_H
#define LLFS_RUNNINGSYSTEM_H

#include <map>
#include "config.h"

using namespace std;
struct RunningSystem {

    struct sys_open_item system_openfiles[SYSOPENFILE];  //ϵͳ�򿪱�
    map<string ,user_open_table*> user_openfiles;        //�û��򿪱���
    struct dir root;                          //rootĿ¼
    hinode hinodes[NHINO];                    //�ڴ�ڵ㻺��
    struct super_block file_system;           //������
    struct PWD pwds[PWDNUM];                  //�û�����

    struct inode *cur_path_inode;             //��ǰĿ¼
    string cur_user;                          //��ǰ�û�
    FILE *disk;                               //ϵͳ�����ļ�
    RunningSystem();
    ~RunningSystem();
    // ���ļ�
    int openFile(const char *pathname, int flags);
    // �ر��ļ�
    void closeFile();
    // ��ȡ�ļ�
    unsigned int readFile();
    // д�ļ�
    unsigned int writeFile();
    // �������ļ�
    inode* createFile(const char *pathname, unsigned short di_mode);
    // ɾ���ļ�
    bool deleteFile();
    // �Ӵ����ļ�����ϵͳ
    void install();
    // ��ʽ��ϵͳ
    void format();
    // �˳�ϵͳ
    void halt();

    //���ݵ�ַѰ�Ҳ����������ڵ�
    struct inode* find_file(char* addr);


    // �û���¼ -1.������� -2.�Ѿ���¼-3.�Ѿ����¼���� >0.��¼�ɹ�(����ֵλ�û��򿪱��±�)
    int login(string pwd);
    // �û�ע��
    void logout(string pwd);

    bool access(string PWD,inode* file_inode);
};



#endif //LLFS_RUNNINGSYSTEM_H
