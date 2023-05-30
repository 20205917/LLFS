#include<stdio.h>
#include<iostream>
#include<string.h>
#include <windows.h>
#include "RunningSystem.h"
using namespace std;


int toUnicode(const char* str)
{
    return str[0] + (str[1] ? toUnicode(str + 1) : 0);
}
constexpr inline int U(const char* str)
{
    return str[0] + (str[1] ? U(str + 1) : 0);
}

// class RunningSystem {

// public:
//     //RunningSystem();
//     //~RunningSystem();
//     // ���ļ�
//     unsigned short openFile();
//     // �ر��ļ�
//     void closeFile();
//     // ��ȡ�ļ�
//     unsigned int readFile();
//     // д�ļ�
//     unsigned int writeFile();
//     // �������ļ�
//     bool createFile();
//     // ɾ���ļ�
//     bool deleteFile();
//     // �Ӵ����ļ�����ϵͳ
//     void install();
//     // ��ʽ��ϵͳ
//     void format();
//     // �˳�ϵͳ
//     void halt();

// };

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

int main(){
    //��ʼ��
    install();
    strcpy(pwds[1].password, "a");
    strcpy(pwds[5].password, "b");
    strcpy(pwds[9].password, "c");
    login("a");
    login("b");
    logout("a");
    login("c");
    logout("b");
    halt();
//    string a;//�����û���������
//    const char* A;
//    string b;//�������
//    string c;//�������
//    string d;//�������
//    int state=0;//������ýӿڷ��񷵻ص�״̬
//    string path=">>";//�������������ǰ��·��
//    system("cls");
//    cout<<"Hello World!"<<endl;
//    cout<<">>";
//    while(1){
//        cin>>a;
//        A=a.data();
//        switch(toUnicode(A)){
//            case U("login")://�û���¼
//                cin>>b;//�����û���
//                //1.��:�Ƿ��û�����Ҫ���������ʾ��Ϣ
//                state=login(b);
//                if(state==-1)
//                    cout<<"�������";
//                else if(state==-2)
//                    cout<<"���û��Ѿ����ڵ�¼״̬";
//                else if(state==-3)
//                    cout<<"�Ѵ��¼����";
//                else
//                    cout<<"��¼�ɹ�";
//                cout<<endl<<path;
//                break;
//
//            case U("logout")://�û��ǳ�
//                cout<<endl<<path;
//                break;
//
//            case U("open")://���ļ�
//                cin>>b;//�����ļ�·��
//                //state=system.openFile();
//                if(state==0)
//                    cout<<"δ�ܳɹ����ļ�";
//                cout<<endl<<path;
//                break;
//
//            case U("close")://�ر��ļ�
//                cin>>b;//�����ļ�·��
//                //state=system.closeFile();����ֵ������
//                cout<<endl<<path;
//                break;
//
//            case U("create")://�����ļ�
//                cin>>b;//�����ļ�·��
//                //state=system.createFile();
//                if(state==0)
//                    cout<<"�����ļ�ʧ��";
//                cout<<endl<<path;
//                break;
//
//            case U("delete")://ɾ���ļ�
//                cin>>b;//�����ļ�·��
//                //state=system.deleteFile();
//                if(state==0)
//                    cout<<"ɾ���ļ�ʧ��";
//                cout<<endl<<path;
//                break;
//
//            case U("write")://д���ļ�
//                cin>>b;//�����ļ���
//                cin>>c;//����д������
//                //state=system.writeFile();
//                if(state==0)
//                    cout<<"δ���ļ������ȴ��ļ���д��";
//                cout<<endl<<path;
//                break;
//
//            case U("read")://���ļ���
//                cin>>b;//�����ļ���
//                cin>>c;//c������Ķ�
//                //state=system.writeFile();
//                if(state==0)
//                    cout<<"δ���ļ������ȴ��ļ��ٶ���";
//                cout<<endl<<path;
//                break;
//
//            case U("format")://���̸�ʽ��
//                //system.format();
//                cout<<endl<<path;
//                break;
//
//            case U("install")://�Ӵ����ļ�����ϵͳ
//                //system.install();
//                cout<<endl<<path;
//                break;
//
//            case U("halt")://�Ӵ����ļ�����ϵͳ
//                //system.halt();
//                cout<<endl<<path;
//                break;
//
//            case U("user")://�鿴�����û�
//                //system.user()
//                cout<<endl<<path;
//                break;
//
//            case U("whoami")://�鿴��ǰ�û�
//                string user=Runningsystem.whoami();
//                cout<<user<<endl<<path;
//                break;
//
//            case U("chmod")://Ȩ�޸���
//                //system.chmod
//                cout<<endl<<path;
//                break;
//
//            default:
//                cout<<"'"<<a<<"'"<<"���ǿ�ִ������"<<endl;
//
//        }
//    }
}
