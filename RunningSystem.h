//
// Created by 86136 on 2023/5/25.
//

#ifndef LLFS_RUNNINGSYSTEM_H
#define LLFS_RUNNINGSYSTEM_H

#include <map>
#include "config.h"
#include "tool.cpp"
#include "memoryManagement.cpp"
//���������operation,��access����������
#define MAKEDIR 1
#define CHDIR   2
#define SHOWDIR 3
#define RMDIR   4
using namespace std;
struct RunningSystem {

    struct sys_open_item system_openfiles[SYSOPENFILE];  //ϵͳ�򿪱�
    map<string, user_open_table*> user_openfiles;        //�û��򿪱���
    struct dir root;                          //rootĿ¼
    hinode hinodes[NHINO];                    //�ڴ�ڵ㻺��
    struct super_block file_system;           //������
    struct PWD pwds[PWDNUM];                  //�û�����

    struct inode *cur_dir_inode;             //��ǰĿ¼���������
    struct dir cur_dir;                       //��ǰĿ¼������
    string cur_user;                          //��ǰ�û�
    FILE *disk;                               //ϵͳ�����ļ�
    RunningSystem();
    ~RunningSystem();
    // ���ļ�
    int openFile(string pathname, unsigned short flags);
    // �ر��ļ�
    void closeFile(const char *pathname);
    // ��ȡ�ļ�
    std::string readFile(const char *pathname);
    // д�ļ�
    // write_mode��ΪW_APPEND W_TRUNC ����������ֵ
    // W_APPEND׷��д W_TRUNC���� ����ֵ��ʾ��ָ��λ��д
    // ����ֵfalseдʧ�� trueд�ɹ�
    bool writeFile(const char *pathname, int write_mode, std::string content);
    // �������ļ�
    inode* createFile(string pathname, unsigned short di_mode);
    // ɾ���ļ�
    bool deleteFile(const char *pathname);
    // �Ӵ����ļ�����ϵͳ
    void install();
    // ��ʽ��ϵͳ
    void format();
    // �˳�ϵͳ
    void halt();

    //���ݵ�ַѰ�Ҳ����������ڵ�
    struct inode* find_file(string addr);


    // �û���¼ -1.������� -2.�Ѿ���¼-3.�Ѿ����¼���� >0.��¼�ɹ�(����ֵΪ�û��򿪱��±�)
    int login(string pwd);
    // �û�ע��
    void logout(string pwd);
    //�ж��û�Ȩ���Ƿ��㹻ĳ����
    bool access(int operation,inode* file_inode);
    // ���ص�ǰ�û�ss
    string whoami();

    // �ļ���·�����
    int mkdir(string pathname);     //�����ļ���
    int chdir(string pathname);     //����ϵͳ�ĵ�ǰ�ļ�·��
    int show_dir();                 //չʾ��ǰ�ļ�·��������
    int rmdir(string pathname);     //ɾ����·���µ��ļ���
    struct dir get_dir(int d_index);//����d_index����ȡdir

    // �ж��Ƿ񱻵�ǰ�û���
    // ���򿪷����û��򿪱��±�
    // δ�򿪷���USER_UNOPENED
    bool isOpened(const char *pathname);

    //�ڴ�ڵ㻺����������
    void exchange_hinodes();

};
struct inode *ialloc(RunningSystem &runningSystem);


#endif //LLFS_RUNNINGSYSTEM_H
