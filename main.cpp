#include<stdio.h>
#include<iostream>
#include<string.h>
#include<string>
#include <windows.h>
#include "RunningSystem.h"
#include<vector>
using namespace std;


bool Split(vector<char*>* token,char* order){
    if(order[0]==' ')
        return false;
    token->push_back(std::strtok(order," "));
    while(token->back()!=NULL){
        token->push_back(std::strtok(NULL," "));
    }
    token->pop_back();
    return true;
}
int toUnicode(const char* str)
{
    return str[0] + (str[1] ? toUnicode(str + 1) : 0);
}
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

        case U("logout"):
            cout<<"logout [PWD]"<<endl;

        case U("create"):
            cout<<"create [pathname][operation]"<<endl;

        case U("delete"):
            cout<<"delete [pathname][operation]"<<endl;

        case U("open"):
            cout<<"open [pathname][operation]"<<endl;

        case U("close"):
            cout<<"close [pathname]"<<endl;

        case U("write"):
            cout<<"write [fd][content]"<<endl;

        case U("read"):
            cout<<"read [fd]"<<endl;

        case U("format"):
            cout<<"format"<<endl;

        case U("help"):
            cout<<"help [order]"<<endl;
        default: cout<<"��ָ�����"<<endl;
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
    int i;
    int state;//״̬
    int fd=-1;//��¼�򿪱�
    //��ʼ��
    install();
    login("admin");
    std::string test = "test";
    mkdir(test);




    string s;//��Ϊstring& �Ĳ���
    vector<char*> token;
    char order[50];
    while(1){
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
                            default: cout<<endl<<"�򿪳ɹ�,�ļ�������Ϊ:"+state<<endl;break;
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
                    cout;
                }
                break;


            case U("create")://�����ļ�
                if(token.size()!=3)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=createFile(std::string(tmp ,std::stoi(token[2])));
                        switch(state){
                            case -1:cout<<endl<<"Ȩ�޲���"<<endl;break;
                            case -2:cout<<endl<<"���ļ����Ѵ���"<<endl;break;
                            case -3:cout<<endl<<"Ŀ¼������"<<endl;break;
                            default: continue;break;
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
                            default: continue;break;
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
                            default: continue;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"���������"<<endl;
                    }
                }
                break;


            case U("read"):
                if(token.size()!=2)
                    cout<<"ָ���ʽ����"<<endl;
                else{
                  try{
                        cout<<readFile(std::stoi(token[1]))<<endl;break;

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
                    chdir(s);
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







        }

    }

}
