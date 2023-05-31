#include<stdio.h>
#include<iostream>
#include<string.h>
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


struct sys_open_item system_openfiles[SYSOPENFILE];  //ϵͳ�򿪱�
map<string, user_open_table*> user_openfiles;        //�û��򿪱���
struct dir root;                          //rootĿ¼
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
    //��ʼ��
    install();
    string path;
    login("admin");
    std::string filename = "test";
    int res = createFile(filename, BUILD_OPEN);

    int fd = open_file(filename, BUILD_OPEN);
    writeFile(fd, "this is a test");
    std::cout << readFile(fd);
    close_file(fd);
    halt();
    return 0;
//    int j = 1;
//    for(int i =0 ;i<102 ; i++ ){
//        j = ialloc(1);
//    }
//    halt();
//    string a;//�����û���������




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
    vector<char*> token;
    char order[50];
    while(1){
        cout<<cur_path;
        token.resize(0);
        cin.get(order,50);
        if(Split(&token,order))
            cout<<endl<<"δ�ҵ�ƥ��ָ��"<<endl<<cur_path;

        switch(toUnicode(token[0])){//ƥ��ָ����
            case U("login")://�û���¼
                if(token.size()!=2)
                    cout<<endl<<"ָ���ʽ����"<<endl<<cur_path;
                else{
                    state=login(std::string(token[1]));
                    if(state==-1)
                        cout<<endl<<"δ�ҵ�ƥ�����"<<endl<<cur_path;
                    else if(state==-2)
                        cout<<endl<<"���û��Ѿ����ڵ�¼״̬"<<endl<<cur_path;
                    else if(state==0)
                        cout<<endl<<"�ɹ���¼"<<endl<<cur_path;
                    else
                        cout<<endl<<"��¼�û��Ѵ�����"<<endl<<cur_path;
                }


            case U("logout")://�û��ǳ�
                if(token.size()!=2)
                    cout<<endl<<"ָ���ʽ����"<<endl<<cur_path;
                else
                    cout<<endl<<"�ɹ��ǳ�"<<endl<<cur_path;
            
            case U("open")://���ļ�
                
                

        }
        
    }

}
