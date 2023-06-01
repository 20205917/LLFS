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
    cout<<"whoami      查看当前用户"<<endl;

    //待补充
    cout<<"help        查看命令含义和格式"<<endl;
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
        default: cout<<"该指令不存在"<<endl;break;
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
    int state;//状态
    //初始化
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



    string s;//作为string& 的参数
    vector<char*> token;
    char order[50];
    while(true){
        cout<<cur_path << ">";
        token.resize(0);
        cin.getline(order, 50);
        if(!Split(&token,order)){
            cout<<"未找到匹配指令"<<endl;
            continue;
        }

        switch(toUnicode(token[0])){//匹配指令名
            case U("login")://用户登录
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    state=login(std::string(token[1]));
                    if(state==-1)
                        cout<<"未找到匹配口令"<<endl;
                    else if(state==-2)
                        cout<<"该用户已经处于登录状态"<<endl;
                    else if(state==0)
                        cout<<"成功登录"<<endl;
                    else
                        cout<<"登录用户已达上限"<<endl;
                }
                break;


            case U("logout")://用户登出
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    logout(std::string(token[1]));
                }
                break;

            case U("open")://打开文件
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=open_file(tmp,std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<endl<<"权限不足"<<endl;break;
                            case -2:cout<<endl<<"未找到文件"<<endl;break;
                            case -3:cout<<endl<<"目录区满"<<endl;break;
                            case -4:cout<<endl<<"未找到空闲系统打开表项"<<endl;break;
                            case -5:cout<<endl<<"未找到空闲用户打开表项"<<endl;break;
                            default: cout<<endl<<"打开成功,文件描述符为:"<<state<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }

                }
                break;

            case U("close")://关闭文件
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    closeFile(std::string(token[1]));
                }
                break;


            case U("create")://创建文件
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=createFile(std::string(tmp));
                        switch(state){
                            case -1:cout<<endl<<"权限不足"<<endl;break;
                            case -2:cout<<endl<<"该文件名已存在"<<endl;break;
                            case -3:cout<<endl<<"目录区已满"<<endl;break;
                            default: continue;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }

                }
                break;


            case U("delete"):
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=deleteFile(tmp);
                        switch(state){
                            case -1:cout<<endl<<"权限不足"<<endl;break;
                            case -2:cout<<endl<<"不存在该文件"<<endl;break;
                            case -3:cout<<endl<<"文件正在被系统打开"<<endl;break;
                            default: continue;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }

                }
                break;


            case U("write"):
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl;
                else{
                  try{
                        state=writeFile(std::stoi(token[1]),std::string(token[2]));
                        switch(state){
                            case 0:cout<<endl<<"读取错误"<<endl;break;
                            default: continue;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }
                }
                break;


            case U("read"):
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl;
                else{
                  try{
                        cout<<readFile(stoi(token[1]),stoi(token[2]))<<endl;break;
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }
                }
                break;

            case U("seek"):
                if(token.size()!=4)
                    cout<<"指令格式错误"<<endl;
                else{
                    try {
                        state = file_seek(stoi(token[1]),stoi(token[2]),stoi(token[3]));
                        if(state<0)
                            cout<<"移动偏移量出界";
                        else
                            cout<<"文件描述符号:"<< stoi(token[1])<<"   offset:"<<state<<endl;
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }
                }
                break;
            case U("format")://格式化
                if(token.size()!=1)
                    cout<<"指令格式错误"<<endl;
                else
                    format();
                break;

            case U("mkdir")://创建目录
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    s=token[1];
                    mkdir(s);
                }
                break;

            case U("cd")://改变目录
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    s=token[1];
                    state = chdir(s);
                    if(state == -1)
                        std::cout << "路径错误" << std::endl;
                    else if(state == -2)
                        std::cout << "路径无效" << std::endl;
                    else if(state == -3)
                        std::cout << "权限不足" << std::endl;

                }
                break;


            case U("rm")://移动目录
                if(token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    s=token[1];
                    rmdir(s);
                }
                break;

            case U("chown")://文件所属用户更改
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=change_file_owner(tmp, std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<"无效路径"<<endl;break;
                            case -2:cout<<"没有修改权限"<<endl;break;
                            default: cout<<"修改成功"<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }
                }
                break;


            case U("chgrp")://文件所在组更改
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl;
                else{
                    try{
                        std::string tmp = std::string(token[1]);
                        state=change_file_group(tmp, std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<"无效路径"<<endl;break;
                            case -2:cout<<"没有修改权限"<<endl;break;
                            default: cout<<"修改成功"<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }
                }
                break;


            case U("usermod")://修改用户所在组
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl;
                else{
                    try{
                        state=usermod(std::stoi(token[1]),std::stoi(token[2]));
                        switch(state){
                            case -1:cout<<"无效uid"<<endl;break;
                            default: cout<<"修改成功"<<endl;break;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }
                }
                break;

            case U("useradd")://在组中新增用户
                if(token.size()!=3)
                    cout<<"指令格式错误"<<endl;
                else{
                    try{
                        state=useradd(std::stoi(token[1]),std::string(token[2]));
                        switch(state){
                            case -1:cout<<"无效uid"<<endl;break;
                            default: cout<<"修改成功"<<endl;
                        }
                    }catch(const std::invalid_argument& e){
                        cout<<"错误操作码"<<endl;
                    }
                }
                break;

            case U("user")://显示所有用户
                if(token.size()!=1)
                    cout<<"指令格式错误";
                else
                    show_all_users();
                cout<<endl;
                break;

            case U("showlogin")://显示登录用户
                if(token.size()!=1)
                    cout<<"指令格式错误";
                else
                    show_login_users();
                cout<<endl;
                break;


            case U("whoami")://查看当前用户
                if(token.size()!=1)
                    cout<<"指令格式错误"<<endl;
                else
                    whoami();
                break;

            case U("show")://展示目录结构
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
                    cout<<"指令格式错误";
                cout<<endl;
                break;



            case U("help")://帮助，打印命令和格式
                if(token.size()!=1 && token.size()!=2)
                    cout<<"指令格式错误"<<endl;
                else{
                    if(token.size() == 1)
                        HelpOut1();
                    else
                        HelpOut2(token[1]);
                }
                break;

            case U("HALT")://保存，退出
                if(token.size()!=1)
                    cout<<"指令格式错误";
                else{
                    halt();
                    std::cout << "系统关闭";
                    exit(1);
                }
                break;
            default: cout<<"未知指令"<<endl;break;
        }

    }

}
