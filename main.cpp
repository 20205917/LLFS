#include<cstdio>
#include<iostream>
#include<cstring>
#include<string>
#include "RunningSystem.h"
#include<vector>
using namespace std;

constexpr inline int U(const char* str)
{
    return str[0] + (str[1] ? U(str + 1) : 0);
}

void HelpOut1(){
    cout<<"�й�ĳ���������ϸ��Ϣ,�����HELP������"<<endl;
    cout<<"login       �û���¼"<<endl;
    cout<<"logout      �û��ǳ�"<<endl;
    cout<<"create      �����ļ�"<<endl;
    cout<<"delete      ɾ���ļ�"<<endl;
    cout<<"open        ���ļ�"<<endl;
    cout<<"close       �ر��ļ�"<<endl;
    cout<<"write       д���ļ�"<<endl;
    cout<<"read        ���ļ���"<<endl;
    cout<<"mkdir       ����Ŀ¼"<<endl;
    cout<<"cd          ���ĵ�ǰĿ¼"<<endl;
    cout<<"rm          �ƶ�Ŀ¼"<<endl;
    cout<<"format      Ӳ�̸�ʽ��"<<endl;
    cout<<"whoami      �鿴��ǰ�û�"<<endl;

    //������
    cout<<"help        �鿴�����͸�ʽ"<<endl;
}

void HelpOut2(char* order){
    switch(toUnicode(order)){
        case U("login"):
            cout<<"login [PWD]"<<endl;
            break;

        case U("logout"):
            cout<<"logout [PWD]"<<endl;
            break;

        case U("create"):
            cout<<"create [pathname][operation]"<<endl;
            break;

        case U("delete"):
            cout<<"delete [pathname][operation]"<<endl;
            break;

        case U("open"):
            cout<<"open [pathname][operation]"<<endl;
            break;

        case U("close"):
            cout<<"close [pathname]"<<endl;
            break;

        case U("write"):
            cout<<"write [fd][content]"<<endl;
            break;

        case U("read"):
            cout<<"read [fd]"<<endl;
            break;

        case U("mkdir"):
            cout<<"mkdir [pathname]"<<endl;
            break;

        case U("cd"):
            cout<<"cd [pathname]"<<endl;
            break;

        case U("rm"):
            cout<<"rm [pathname]"<<endl;
            break;

        case U("chown"):
            cout<<"chown [pathname][uid]"<<endl;
            break;

        case U("chgrp"):
            cout<<"chgrp [pathname][gid]"<<endl;
            break;

        case U("usermod"):
            cout<<"usermod [uid][gid]"<<endl;
            break;

        case U("useradd"):
            cout<<"useradd [gid][PWD]"<<endl;
            break;


        case U("show"):
            cout<<"show dir"<<endl;
            cout<<"show file"<<endl;
            cout<<"show dir all"<<endl;
            cout<<"show user all"<<endl;
            cout<<"show user file"<<endl;
            cout<<"show sys all"<<endl;
            break;

        case U("format"):
            cout<<"format"<<endl;
            break;

        case U("help"):
            cout<<"help [order]"<<endl;
            break;
        default: cout<<"��ָ�����"<<endl;break;
    }
}
struct sys_open_item system_openfiles[SYSOPENFILE];  //ϵͳ�򿪱�
map<string, user_open_table*> user_openfiles;        //�û��򿪱���
hinode hinodes[NHINO];                    //�ڴ�ڵ㻺��
struct super_block file_system;           //������
struct PWD pwds[PWDNUM];                  //�û�����

struct inode *cur_dir_inode;             //��ǰĿ¼���������
string cur_user;                          //��ǰ�û�
FILE *disk;                               //ϵͳ�����ļ�
string cur_path;                          // ��ǰ·����


int main(){
    int state;//״̬
    //��ʼ��
    initial();
    install();
    login("admin");
    std::string test = "test";
    std::string ok = "ok";
    std::string root = "root";
    mkdir(test);
    chdir(test);
    mkdir(ok);
    chdir(root);
    createFile("1.1");
    root = "1.1";
    writeFile(open_file(root,1),"aaaabbbb1111");



    string s;//��Ϊstring& �Ĳ���
    vector<char*> token;
    char order[50];
    while(true){
        cout<<cur_path << ">";
        token.resize(0);
        cin.getline(order, 50);
        if(!Split(&token,order)){
            cout<<"δ�ҵ�ƥ��ָ��"<<endl;
            continue;
        }

        switch(toUnicode(token[0])){//ƥ��ָ����
            case U("login")://�û���¼
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    state=login(std::string(token[1]));
                    if(state==-1)
                        cout<<"δ�ҵ�ƥ�����"<<endl;
                    else if(state==-2)
                        cout<<"���û��Ѿ����ڵ�¼״̬"<<endl;
                    else if(state==0)
                        cout<<"�ɹ���¼"<<endl;
                    else
                        cout<<"��¼�û��Ѵ�����"<<endl;
                }
                break;


            case U("logout")://�û��ǳ�
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    logout(std::string(token[1]));
                }
                break;

            case U("open")://���ļ�
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=open_file(tmp,std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<endl<<"Ȩ�޲���"<<endl;break;
                            case -2:cout<<endl<<"δ�ҵ��ļ�"<<endl;break;
                            case -3:cout<<endl<<"Ŀ¼����"<<endl;break;
                            case -4:cout<<endl<<"δ�ҵ�����ϵͳ�򿪱���"<<endl;break;
                            case -5:cout<<endl<<"δ�ҵ������û��򿪱���"<<endl;break;
                            default: cout<<endl<<"�򿪳ɹ�,�ļ�������Ϊ:"<<state<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }

                }
                break;

            case U("close")://�ر��ļ�
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    closeFile(std::string(token[1]));
                }
                break;


            case U("create")://�����ļ�
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=createFile(std::string(tmp));
                        switch(state){
                            case -1:cout<<endl<<"Ȩ�޲���"<<endl;break;
                            case -2:cout<<endl<<"���ļ����Ѵ���"<<endl;break;
                            case -3:cout<<endl<<"Ŀ¼������"<<endl;break;
                            default: continue;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }

                }
                break;


            case U("delete"):
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=deleteFile(tmp);
                        switch(state){
                            case -1:cout<<endl<<"Ȩ�޲���"<<endl;break;
                            case -2:cout<<endl<<"�����ڸ��ļ�"<<endl;break;
                            case -3:cout<<endl<<"�ļ����ڱ�ϵͳ��"<<endl;break;
                            default: continue;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }

                }
                break;


            case U("write"):
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                  try{
                        state=writeFile(std::stoi(token[1]),std::string(token[2]));
                        switch(state){
                            case 0:cout<<endl<<"��ȡ����"<<endl;break;
                            default: continue;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;


            case U("read"):
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                  try{
                        cout<<readFile(stoi(token[1]),stoi(token[2]))<<endl;break;
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;

            case U("seek"):
                if(token.size()!=4)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try {
                        state = file_seek(stoi(token[1]),stoi(token[2]),stoi(token[3]));
                        if(state<0)
                            cout<<"�ƶ�ƫ��������";
                        else
                            cout<<"�ļ���������:"<< stoi(token[1])<<"   offset:"<<state<<endl;
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;
            case U("format")://��ʽ��
                if(token.size()!=1)
                    cout<<"ָ���ʽ����"<<endl;
                else
                    format();
                break;

            case U("mkdir")://����Ŀ¼
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    s=token[1];
                    mkdir(s);
                }
                break;

            case U("cd")://�ı�Ŀ¼
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    s=token[1];
                    state = chdir(s);
                    if(state == -1)
                        std::cout << "·������" << std::endl;
                    else if(state == -2)
                        std::cout << "·����Ч" << std::endl;
                    else if(state == -3)
                        std::cout << "Ȩ�޲���" << std::endl;

                }
                break;


            case U("rm")://�ƶ�Ŀ¼
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    s=token[1];
                    rmdir(s);
                }
                break;

            case U("chown")://�ļ������û�����
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=change_file_owner(tmp, std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<"��Ч·��"<<endl;break;
                            case -2:cout<<"û���޸�Ȩ��"<<endl;break;
                            default: cout<<"�޸ĳɹ�"<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;


            case U("chgrp")://�ļ����������
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=change_file_group(tmp, std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<"��Ч·��"<<endl;break;
                            case -2:cout<<"û���޸�Ȩ��"<<endl;break;
                            default: cout<<"�޸ĳɹ�"<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;


            case U("usermod")://�޸��û�������
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        state=usermod(std::stoi(token[1]),std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<"��Чuid"<<endl;break;
                            default: cout<<"�޸ĳɹ�"<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;

            case U("useradd")://�����������û�
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        state=useradd(std::stoi(token[1]),std::string(token[2]));
                        switch(state){
                            case -1:cout<<"��Чuid"<<endl;break;
                            default: cout<<"�޸ĳɹ�"<<endl;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;

            case U("user")://��ʾ�����û�
                if(token.size()!=1)
                    cout<<"ָ���ʽ����";
                else
                    show_all_users();
                cout<<endl;
                break;

            case U("showlogin")://��ʾ��¼�û�
                if(token.size()!=1)
                    cout<<"ָ���ʽ����";
                else
                    show_login_users();
                cout<<endl;
                break;


            case U("whoami")://�鿴��ǰ�û�
                if(token.size()!=1)
                    cout<<"ָ���ʽ����"<<endl;
                else
                    whoami();
                break;

            case U("show")://չʾĿ¼�ṹ
                if(token.size()==1)
                    show_dir(); // dir ls
                else if(token.size()==2 && !strcmp(token[1],"all"))
                    show_whole_dir();
                else if(token.size()==3 && !strcmp(token[1],"user")&&!strcmp(token[2],"login"))
                    show_login_users();
                else if(token.size()==3 && !strcmp(token[1],"user")&&!strcmp(token[2],"all"))
                    show_all_users();
                else if(token.size()==3 && !strcmp(token[1],"user")&&!strcmp(token[2],"file"))
                    show_user_opened_files();
                else if(token.size()==3 && !strcmp(token[1],"sys")&&!strcmp(token[2],"file"))
                    show_sys_opened_files();
                else
                    cout<<"ָ���ʽ����";
                cout<<endl;
                break;



            case U("help")://��������ӡ����͸�ʽ
                if(token.size()!=1 && token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    if(token.size() == 1)
                        HelpOut1();
                    else
                        HelpOut2(token[1]);
                }
                break;

            case U("HALT")://���棬�˳�
                if(token.size()!=1)
                    cout<<"ָ���ʽ����";
                else{
                    halt();
                    std::cout << "ϵͳ�ر�";
                    exit(1);
                }
                break;
            default: cout<<"δָ֪��"<<endl;break;
        }

    }

}
