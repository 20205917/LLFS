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

extern hinode hinodes[NHINO];                    //�ڴ�ڵ㻺��
extern struct super_block file_system;           //������
extern struct PWD pwds[PWDNUM];                  //�û�����

extern struct inode *cur_dir_inode;             //��ǰĿ¼���������
extern string cur_user;                          //��ǰ�û�
extern string cur_path;                        //��ǰĿ¼��
// ���ڷ�����2+DINODEBLK+FILEBLK�����̿�
// ǰ2��Ϊ�����죨���ڷ����û���Ϣ����������
// ��������DINODEBLK���飬������FILEBLK����
extern FILE *disk;                               //ϵͳ�����ļ�

    int hard_link(string &pathname,string &newname);
    // ���ļ�
    int openFile(const string& pathname, unsigned short flags);
    int open_file(string& pathname, int operation);
    // �ر��ļ�
    void closeFile(const string& pathname);
    int close_file(int fd);
    // ��ȡ�ļ�
    string readFile(int fd);
    /* д�ļ�
     write_mode��ΪW_APPEND W_TRUNC ����������ֵ
     W_APPEND׷��д W_TRUNC���� ����ֵ��ʾ��ָ��λ��д
     ����ֵfalseдʧ�� trueд�ɹ�
     */
    bool writeFile(const string& pathname, int write_mode, const string& content);
    bool writeFile(int fd, const string& content);
    // �������ļ�
    int createFile(string pathname);
    // ɾ���ļ�
    int deleteFile(string pathname);
    // ��ʼ��
    void initial();
    // �Ӵ����ļ�����ϵͳ
    void install();
    // ��ʽ��ϵͳ
    void format();
    // �˳�ϵͳ
    void halt();

    //���ݵ�ַѰ�Ҳ����������ڵ�
    struct inode* find_file(string addr);
    // �������i�ڵ�
    struct inode *ialloc();
    // �û���¼ -1.������� -2.�Ѿ���¼-3.�Ѿ����¼���� >0.��¼�ɹ�(����ֵΪ�û��򿪱��±�)
    int login(const string& pwd);
    // �û�ע��
    void logout(const string& pwd);
    //�ж��û�Ȩ���Ƿ��㹻ĳ����
    bool access(int operation, inode *file_inode);
    // ���ص�ǰ�û�
    string whoami();
    void show_all_users();
    void show_login_users();
    int switch_user(const string& pwd);
    int usermod(int uid, int gid);
    int useradd(int gid, const std::string& pwd);

    // �ļ���·�����
    int mkdir(string& pathname);     //�����ļ���
    int chdir(string& pathname);     //����ϵͳ�ĵ�ǰ�ļ�·��
    int show_dir();                 //չʾ��ǰ�ļ�·��������
    int show_whole_dir();           // չʾ�ļ�ϵͳ����Ŀ¼�ṹ
    int show_dir_tree(unsigned int id, int depth);
    int rmdir(string& pathname);     //ɾ����·���µ��ļ���
    struct dir get_dir(unsigned int d_index);//����d_index����ȡdir

    //�ļ��������
    int change_file_owner(string& pathname, int uid);   //�ı��ļ������û�
    int change_file_group(string& pathname, int gid);   //�ı��ļ�������

    // �ж��Ƿ񱻵�ǰ�û���,���򿪷����û��򿪱��±�,δ�򿪷���USER_UNOPENED
    bool isOpened(string pathname);

    int seek_catalog_leisure();
    // ����i�ڵ����
    int ialloc(unsigned int);

    void ifree(unsigned int dinode_id);

    inode* iget(unsigned int dinode_id);

    bool iput(inode* inode);

    void bfree(int block_num);

    unsigned int balloc();

    unsigned int namei(string name);

    void file_wirte_back(struct inode* inode);

    inode* getDinodeFromDisk(int dinode_id);

//�鿴ĳ������i�ڵ�id��Ӧ���ڴ�i�ڵ��Ƿ����
    inode* findHinode(int dinode_id);

#endif //LLFS_RUNNINGSYSTEM_H