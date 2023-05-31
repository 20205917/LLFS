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
    cout<<"有关某个命令的详细信息,请键入HELP命令名"<<endl;
    cout<<"login       用户登录"<<endl;
    cout<<"logout      用户登出"<<endl;
    cout<<"create      创建文件"<<endl;
    cout<<"delete      删除文件"<<endl;
    cout<<"open        打开文件"<<endl;
    cout<<"close       关闭文件"<<endl;
    cout<<"write       写入文件"<<endl;
    cout<<"read        从文件读"<<endl;
    cout<<"mkdir       创建目录"<<endl;
    cout<<"cd          更改当前目录"<<endl;
    cout<<"rm          移动目录"<<endl;
    cout<<"format      硬盘格式化"<<endl;
    //待补充
    cout<<"help        查看命令含义和格式"<<endl;
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
        default: cout<<"该指令不存在"<<endl;
    }
}


struct sys_open_item system_openfiles[SYSOPENFILE];  //系统打开表
map<string, user_open_table*> user_openfiles;        //用户打开表组
hinode hinodes[NHINO];                    //内存节点缓存
struct super_block file_system;           //超级块
struct PWD pwds[PWDNUM];                  //用户数组

struct inode *cur_dir_inode;             //当前目录的索引结点
string cur_user;                          //当前用户
FILE *disk;                               //系统磁盘文件
string cur_path;                          // 当前路径名


int main(){
    int i;
    int state;//状态
    int fd=-1;//记录打开表
    //初始化
    install();
    string path;
    login("admin");
    string file = "/aaa.txt";
    string dirname2 = "bbb";
    createFile(file);
    mkdir(dirname2);
    show_dir();
    std::cout << std::endl;
    string new_name = "b.txt";
    hard_link(file,new_name);
    show_dir();
    show_whole_dir();

    return 0;
//    int j = 1;
//    for(int i =0 ;i<102 ; i++ ){
//        j = ialloc(1);
//    }
//    halt();
//    string a;//接收用户输入命令




//    string a;//接收用户输入命令
//    const char* A;
//    string b;//命令参数
//    string c;//命令参数
//    string d;//命令参数
//    int state=0;//保存调用接口服务返回的状态
//    string path=">>";//命令窗口输入命令前的路径
//    system("cls");
//    cout<<"Hello World!"<<endl;
//    cout<<">>";
//    while(1){
//        cin>>a;
//        A=a.data();
//        switch(toUnicode(A)){
//            case U("login")://用户登录
//                cin>>b;//输入用户名
//                //1.补:非法用户名需要输出错误提示信息
//                state=login(b);
//                if(state==-1)
//                    cout<<"口令错误";
//                else if(state==-2)
//                    cout<<"该用户已经处于登录状态";
//                else if(state==-3)
//                    cout<<"已达登录上限";
//                else
//                    cout<<"登录成功";
//                cout<<endl<<path;
//                break;
//
//            case U("logout")://用户登出
//                cout<<endl<<path;
//                break;
//
//            case U("open")://打开文件
//                cin>>b;//输入文件路径
//                //state=system.openFile();
//                if(state==0)
//                    cout<<"未能成功打开文件";
//                cout<<endl<<path;
//                break;
//
//            case U("close")://关闭文件
//                cin>>b;//输入文件路径
//                //state=system.closeFile();返回值有问题
//                cout<<endl<<path;
//                break;
//
//            case U("create")://创建文件
//                cin>>b;//输入文件路径
//                //state=system.createFile();
//                if(state==0)
//                    cout<<"创建文件失败";
//                cout<<endl<<path;
//                break;
//
//            case U("delete")://删除文件
//                cin>>b;//输入文件路径
//                //state=system.deleteFile();
//                if(state==0)
//                    cout<<"删除文件失败";
//                cout<<endl<<path;
//                break;
//
//            case U("write")://写入文件
//                cin>>b;//输入文件名
//                cin>>c;//输入写入内容
//                //state=system.writeFile();
//                if(state==0)
//                    cout<<"未打开文件，请先打开文件再写入";
//                cout<<endl<<path;
//                break;
//
//            case U("read")://从文件读
//                cin>>b;//输入文件名
//                cin>>c;//c代表从哪读
//                //state=system.writeFile();
//                if(state==0)
//                    cout<<"未打开文件，请先打开文件再读出";
//                cout<<endl<<path;
//                break;
//
//            case U("format")://磁盘格式化
//                //system.format();
//                cout<<endl<<path;
//                break;
//
//            case U("install")://从磁盘文件加载系统
//                //system.install();
//                cout<<endl<<path;
//                break;
//
//            case U("halt")://从磁盘文件加载系统
//                //system.halt();
//                cout<<endl<<path;
//                break;
//
//            case U("user")://查看所有用户
//                //system.user()
//                cout<<endl<<path;
//                break;
//
//            case U("whoami")://查看当前用户
//                string user=Runningsystem.whoami();
//                cout<<user<<endl<<path;
//                break;
//
//            case U("chmod")://权限更改
//                //system.chmod
//                cout<<endl<<path;
//                break;
//
//            default:
//                cout<<"'"<<a<<"'"<<"不是可执行命令"<<endl;
//
//        }
//    }


}

    vector<char*> token;
    char order[50];
    while(1){
        cout<<cur_path;
        token.resize(0);
        cin.get(order,50);
        if(Split(&token,order))
            cout<<"未找到匹配指令"<<endl<<cur_path;

        switch(toUnicode(token[0])){//匹配指令名
            case U("login")://用户登录
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                    state=login(std::string(token[1]));
                    if(state==-1)
                        cout<<"未找到匹配口令"<<endl<<cur_path;
                    else if(state==-2)
                        cout<<"该用户已经处于登录状态"<<endl<<cur_path;
                    else if(state==0)
                        cout<<"成功登录"<<endl<<cur_path;
                    else
                        cout<<"登录用户已达上限"<<endl<<cur_path;
                }
                break;


            case U("logout")://用户登出
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                    logout(std::string(token[1]));
                    cout<<cur_path;
                }
                break;
            
            case U("open")://打开文件
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                    try{
                        state=open_file(std::string(token[1]),std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<endl<<"权限不足"<<endl<<cur_path;break;
                            case -2:cout<<endl<<"未找到文件"<<endl<<cur_path;break;
                            case -3:cout<<endl<<"目录区满"<<endl<<cur_path;break;
                            case -4:cout<<endl<<"未找到空闲系统打开表项"<<endl<<cur_path;break;
                            case -5:cout<<endl<<"未找到空闲用户打开表项"<<endl<<cur_path;break;
                            default: cout<<endl<<"打开成功,文件描述符为:"+state<<endl<<cur_path;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl<<cur_path;
                    }
 
                }
                break;

            case U("close")://关闭文件
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                    closeFile(std::string(token[1]));
                    cout<<cur_path;
                }
                break;


            case U("create")://创建文件
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                    try{
                        state=createFile(std::string(token[1]),std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<endl<<"权限不足"<<endl<<cur_path;break;
                            case -2:cout<<endl<<"该文件名已存在"<<endl<<cur_path;break;
                            case -3:cout<<endl<<"目录区已满"<<endl<<cur_path;break;
                            default: continue;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl<<cur_path;
                    }

                }
                break;
            

            case U("delete"):
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                    try{
                        state=deleteFile(std::string(token[1]),std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<endl<<"权限不足"<<endl<<cur_path;break;
                            case -2:cout<<endl<<"不存在该文件"<<endl<<cur_path;break;
                            case -3:cout<<endl<<"文件正在被系统打开"<<endl<<cur_path;break;
                            default: continue;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl<<cur_path;
                    }

                }
                break;


            case U("write"):
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                  try{
                        state=writeFile(std::stoi(token[1]),std::string(token[2]));
                        switch(state){
                            case 0:cout<<endl<<"读取错误"<<endl<<cur_path;break;
                            default: continue;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl<<cur_path;
                    }
                }
                break;


            case U("read"):
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                  try{
                        cout<<readFile(std::stoi(token[1]))<<endl<<cur_path;break;

                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl<<cur_path;
                    }        
                }
                break;

            
            case U("format")://格式化
                if(token.size()!=1)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else
                    format();
                break;

            case U("help")://帮助，打印命令和格式
                if(token.size()!=1||token.size()!=2)
                    cout<<"指令格式错误"<<endl<<cur_path;
                else{
                    if(token.size()!=1)
                        HelpOut1();
                    else
                        HelpOut2(token[1]);
                }
                cout<<cur_path;
                break;
                        

            
            
                
                

        }
        
    }

}
