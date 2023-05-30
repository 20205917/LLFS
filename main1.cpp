#include<stdio.h>
#include<iostream>
#include<string.h>
#include <windows.h>
#include "RunningSystem.h"
using namespace std;

int main(){
    string a;//接收用户输入命令
    const char* A;
    string b;//命令参数
    string c;//命令参数
    string d;//命令参数
    int state=0;//保存调用接口服务返回的状态
    string path=">>";//命令窗口输入命令前的路径
    system("cls");
    cout<<"Hello World!"<<endl;
    cout<<">>";
    while(1){
        cin>>a;
        A=a.data();
        switch(toUnicode(A)){
            case U("login")://用户登录
                cin>>b;//输入用户名
                //1.补:非法用户名需要输出错误提示信息
                state=Runningsystem.login(b);
                if(state==-1)
                    cout<<"口令错误";
                else if(state==-2)
                    cout<<"该用户已经处于登录状态";
                else if(state==-3)
                    cout<<"已达登录上限";
                else
                    cout<<"登录成功";
                cout<<endl<<path;
                break;

            case U("logout")://用户登出
                cout<<endl<<path;
                break;

            case U("open")://打开文件
                cin>>b;//输入文件路径
                //state=system.openFile();
                if(state==0)
                    cout<<"未能成功打开文件";
                cout<<endl<<path;
                break;

            case U("close")://关闭文件
                cin>>b;//输入文件路径
                //state=system.closeFile();返回值有问题
                cout<<endl<<path;
                break;

            case U("create")://创建文件
                cin>>b;//输入文件路径
                //state=system.createFile();
                if(state==0)
                    cout<<"创建文件失败";
                cout<<endl<<path;
                break;
            
            case U("delete")://删除文件
                cin>>b;//输入文件路径
                //state=system.deleteFile();
                if(state==0)
                    cout<<"删除文件失败";
                cout<<endl<<path;
                break;

            case U("write")://写入文件
                cin>>b;//输入文件名
                cin>>c;//输入写入内容
                //state=system.writeFile();
                if(state==0)
                    cout<<"未打开文件，请先打开文件再写入";
                cout<<endl<<path;
                break;

            case U("read")://从文件读
                cin>>b;//输入文件名
                cin>>c;//c代表从哪读
                //state=system.writeFile();
                if(state==0)
                    cout<<"未打开文件，请先打开文件再读出";
                cout<<endl<<path;
                break;

            case U("format")://磁盘格式化
                //system.format();
                cout<<endl<<path;
                break;

            case U("install")://从磁盘文件加载系统
                //system.install();
                cout<<endl<<path;
                break;

            case U("halt")://从磁盘文件加载系统
                //system.halt();
                cout<<endl<<path;
                break;   

            case U("user")://查看所有用户
                //system.user()
                cout<<endl<<path;
                break;

             case U("whoami")://查看当前用户
                string user=Runningsystem.whoami();
                cout<<user<<endl<<path;
                break;

            case U("chmod")://权限更改
                //system.chmod
                cout<<endl<<path;
                break;

            default:
                cout<<"'"<<a<<"'"<<"不是可执行命令"<<endl;

        }
    }
}
