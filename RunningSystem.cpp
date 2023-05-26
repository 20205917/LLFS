//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include "RunningSystem.h"

RunningSystem::RunningSystem(){
    // 读硬盘
    disk = fopen("disk", "wb+");

    // 初始化file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // 初始化hinodes
    for(int i = 0; i < NHINO; i++){
        hinodes[i] = (hinode)malloc(sizeof(struct inode));
        hinodes[i]->i_forw = nullptr;
    }

    // 初始化system_openfiles
    for(int i = 0; i < SYSOPENFILE; i++){
        system_openfiles[i].i_count = 0;
        system_openfiles[i].fcb.d_ino = 0;
    }

    // 初始化pwds
    fread(&pwds, sizeof(PWD), PWDNUM, disk);

    // 初始化user_openfiles
    user_openfiles.clear();


    // 读取root目录
    // 即第一个i节点
    // 初始化cur_path_inode
    cur_path_inode = iget(1, hinodes, disk);
    int size = cur_path_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        id = cur_path_inode->dinode.di_addr[i];
        addr = DINODESTART + id * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fread(&root+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_path_inode->dinode.di_addr[block_num];
    addr = DINODESTART + id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread(&root+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);

};

int  RunningSystem::login(string pwd){
    int i,j;
    //检查是否有匹配的PWD
    for(i = 0; i  < PWDNUM &&strcmp(pwd.c_str(),pwds[PWDNUM].password); i++);
    if(i==PWDNUM)
        return -1;

    //检查是否已经登录
    if(user_openfiles.find(pwd) != user_openfiles.end()){
        return -2;
    }

    //是否还有空位
    if(user_openfiles.size()<USERNUM){
        user_open_table* openTable = new user_open_table;
        openTable->p_uid = pwds[i].p_uid;
        openTable->p_gid = pwds[i].p_gid;
        memset(openTable->items,0,sizeof (user_open_item) * NOFILE);
        user_openfiles.insert(pair<string,user_open_table*>(pwd,openTable));
        //设置当前文件为根目录

        //cur_path_inode = iget(1);
    }
    return -3;
}
void RunningSystem::logout(string pwd){
    user_open_table* u = user_openfiles.find(pwd)->second;
    for (int i = 0; i < NOFILE; ++i) {
        //关闭每个文件
        unsigned short id_to_sysopen = u->items[i].id_to_sysopen;
        if(--system_openfiles[id_to_sysopen].i_count == 0){
            iput(system_openfiles[id_to_sysopen].fcb.d_ino,disk,file_system);
        };
    }
    user_openfiles.erase(pwd);
    return;
}



//判断基础合法性，存在，长度，位于根目录
//清理多余合法符号
//具体待修改.(暂定）
char *clean_path(const char *path){

    if (path == NULL || strlen(path) < 1 || strlen(path) > 1024 || path[0] != '/')
        return NULL;

    for (int i = 0; i < strlen(path); i++) {
        if (!((path[i] >= '0' && path[i] <= '9')
              ||(path[i] >= 'a' && path[i] <= 'z')
              || (path[i] >= 'A' && path[i] <= 'Z')
              ||path[i] == '.' || path[i] == '/'))    return NULL;
    }
    char *old_path = (char *) malloc(strlen(path) + 1);
    strcpy(old_path, path);
    char *new_path = (char *) malloc(strlen(path) + 1);
    memset(new_path, 0, strlen(path) + 1);
    int i = 0;

    int k = 0;
//    单个文件和目录名长度 <= 32 字节
    char *temp = strtok(old_path, "/");
    while (temp != NULL) {
        if (strlen(temp) > 32) {
            k = 1;
            break;
        }
        new_path[i++] = '/';
        for(int j = 0;j < strlen(temp); j++)
            new_path[i++] = temp[j];
        temp = strtok(NULL, "/");
    }
    free(old_path);
    if(k == 1){
        //free(new_path);
        new_path =NULL;
        return NULL;
    }
    return new_path;
}

int RunningSystem::openFile(const char *pathname, int flags) {
    //判断合法性
    char *path = NULL;
    //clean_path清除
    if ((path = clean_path(pathname)) == NULL)
        return -1;
    //寻找文件
    inode* in = find_file(path);

    if(in == nullptr){
        free(path);
        return -2;
    }
    //存在则查找访问权限
    if(access(cur_user, in)){
        free(path);
        return -3;
    }
    iget(1,hinodes,disk);

    struct user_open_item{
        unsigned int f_count;               //使用进程数
        unsigned short u_default_mode;      //打开方式
        struct inode *f_inode;              //内存i节点指针
        unsigned long f_offset;             //文件偏移量（文件指针）
        unsigned short id_to_sysopen;       //系统打开表索引
    };

    user_open_table* u = user_openfiles.find(cur_user)->second;
    int fd = 0;
    while (u->items[++fd]. != NULL) ;

    return fd;
}
//创建文件夹，输入是文件路径和文件夹名
bool RunningSystem::mkdir(const char *pathname, char *name)
{
    
    return false;
}

//显示当前用户ss
string RunningSystem::whoami(){
    return cur_user;
}
RunningSystem::~RunningSystem(){
    fclose(disk);
}

