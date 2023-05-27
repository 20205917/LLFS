//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include "RunningSystem.h"



RunningSystem::RunningSystem(){
    // 读硬盘
    disk = fopen("disk", "rb+");

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
        system_openfiles[i].fcb.d_index = 0;
    }

    // 初始化pwds
    fread(&pwds, sizeof(PWD), PWDNUM, disk);

    // 初始化user_openfiles
    user_openfiles.clear();


    // 读取root目录和cur_dir当前目录
    // 即第一个i节点
    // 初始化cur_dir_inode
    cur_dir_inode = iget(1, hinodes, disk);
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DINODESTART + id * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char*)(&root)+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
        fread((char*)(&cur_dir)+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DINODESTART + id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char*)(&root)+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);
    fread((char*)(&cur_dir)+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);

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
            iput(system_openfiles[id_to_sysopen].fcb.d_index,disk,file_system);
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
struct dir RunningSystem::get_dir(int d_index)
{
    inode *dir_inode = iget(d_index,hinodes,disk);
    // 从磁盘加载目录
    struct dir work_dir;
    work_dir.size=dir_inode->dinode.di_size/(DIRSIZ+2);
    int i=0;
    for(i= 0; i<work_dir.size/(BLOCKSIZ/(DIRSIZ+2));i++)
    {
        fseek(disk,DATASTART+BLOCKSIZ * dir_inode->dinode.di_addr[i],SEEK_SET);
        fread(&work_dir.files[(BLOCKSIZ/(DIRSIZ+2)) * i],1, BLOCKSIZ,disk);
    }
    fseek(disk, DATASTART+BLOCKSIZ * dir_inode->dinode.di_addr[i],SEEK_SET);
    fread(&work_dir.files[(BLOCKSIZ)/(DIRSIZ+2) * i], 1,dir_inode->dinode.di_size % BLOCKSIZ, disk);
    return work_dir;
}
// 创建文件夹，输入是文件路径和文件夹名
int RunningSystem::mkdir(string pathname, char *name)
{
    if(!is_dir(pathname)){
        return false;
    }
    else{
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0,pos-1);
        string file = pathname.substr(pos);
        inode* catalog =  find_file(pathname);
        //判断目录文件数据区是否有空闲
        if(seek_catalog_leisure(catalog,disk)!=-1){
            //判断是否重复
            
        }
        else{//没有空闲，失败
            return -1;
        }
    }
}


//显示当前用户ss
string RunningSystem::whoami(){
    return cur_user;
}

RunningSystem::~RunningSystem(){
    fclose(disk);
}
// 目录文件数据区是否有空闲，是否在父目录数据区里有已存在文件(findfile()实现)，有空闲
// 申请磁盘i结点，初始化磁盘i结点，磁盘i结点写回磁盘，父目录数据区写回磁盘。
// 申请磁盘i节点未实现
inode* RunningSystem::createFile(const char *pathname, unsigned short di_mode)
{
    // 判断文件名是否合法
    if(!is_file(pathname)){
        return nullptr;
    }
    hinode res_inode = (hinode)malloc(sizeof(struct inode));
    res_inode->d_index = 0;
    // 是否在父目录数据区里有已存在文件
    if(find_file(const_cast<char *>(pathname))->d_index != 0){
        return res_inode;
    }
    // 是否有空闲
    int index = iname(const_cast<char *>(pathname), cur_dir_inode, disk);
    if(index == DIRNUM)
        return nullptr;
    // 申请磁盘i结点
    struct inode* new_inode = ialloc();
    // 初始化磁盘i节点
    new_inode->dinode.di_mode = di_mode;
    new_inode->dinode.di_size = 0;
    // 磁盘i节点写回磁盘
    long addr = DINODESTART + new_inode->d_index * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(new_inode->dinode), DINODESIZ, 1, disk);
    // 新增目录项
    cur_dir.files[index].d_index = new_inode->d_index;
    cur_dir.size++;
    // 父目录写回磁盘数据区
    write_data_back((void*)(&cur_dir), cur_dir_inode->dinode.di_addr, sizeof(struct dir), disk);
    return new_inode;
}

// 文件名是否合法，需要findfile，判断是否有权限
// 修改父目录数据区并写入磁盘，iput()删除文件
// false删除失败 true删除成功
// 权限未实现 iput未实现
bool RunningSystem::deleteFile(const char *pathname){
    // 判断文件名是否合法
    if(!is_file(pathname)){
        return false;
    }
    hinode res_inode = find_file(const_cast<char *>(pathname));
    // 是否在父目录数据区里有该文件
    if(res_inode->d_index == 0){
        return false;
    }
    // 判断用户对父目录有写权限和执行权限是否
    // if(access())

    // 修改父目录数据区
    // 更改目录项
    unsigned int index = namei(const_cast<char *>(pathname), cur_dir_inode, disk);
    cur_dir.files[index].d_index = 0;
    cur_dir.size--;
    // 写入磁盘
    write_data_back((void*)(&cur_dir), cur_dir_inode->dinode.di_addr, sizeof(struct dir), disk);
    // iput()删除文件
    // iput(res_inode);

    return true;
}

// 关闭一个已经被用户打开了的文件

void RunningSystem::closeFile(const char *pathname){
    // 判断文件名是否合法
    if(!is_file(pathname)){
        return;
    }
    // 获取用户的打开表
    user_open_table* userOpenTable = user_openfiles[cur_user];
    // 获取用户uid
    unsigned short p_uid = userOpenTable->p_uid;

    // 遍历查询该文件
    unsigned short id;
    bool found = false;
    int i;
    for(i = 0; i < NOFILE; i++){
        if(userOpenTable->items[i].f_inode == nullptr){
            continue;
        }
        id = userOpenTable->items[i].id_to_sysopen;
        if(!strcmp(system_openfiles[id].fcb.d_name, pathname) && system_openfiles[id].i_count != 0){
            found = true;
            break;
        }
    }
    // 用户没有打开该文件
    if(!found)
        return;

    // 确定打开了
    // 释放该文件的内存i节点
    iput(userOpenTable->items[i].f_inode, disk, file_system);
    userOpenTable->items[i].f_inode = nullptr;
    // 系统打开表判断是否需要关闭
    system_openfiles[id].i_count--;
    if(system_openfiles[id].i_count == 0){
        // 将这个文件从系统打开表中关闭
        system_openfiles[id].fcb.d_index = 0;
    }
}

// 从用户以打开的文件中读取内容
// 以字符形式返回内容
// 没有实现权限判断
std::string RunningSystem::readFile(const char *pathname){
    // 判断文件名是否合法
    if(!is_file(pathname)){
        return {};
    }

    // 判断用户对该文件是否有读权限
    // if(access())

    // 判断文件是否被用户打开
    // 获取用户的打开表
    user_open_table* userOpenTable = user_openfiles[cur_user];
    // 获取用户uid
    unsigned short p_uid = userOpenTable->p_uid;

    // 遍历查询该文件
    unsigned short id;
    bool found = false;
    int i;
    for(i = 0; i < NOFILE; i++){
        if(userOpenTable->items[i].f_inode == nullptr){
            continue;
        }
        id = userOpenTable->items[i].id_to_sysopen;
        if(!strcmp(system_openfiles[id].fcb.d_name, pathname) && system_openfiles[id].i_count != 0){
            found = true;
            break;
        }
    }
    // 用户没有打开该文件
    if(!found)
        return {};

    // 确定打开了
    unsigned int dinode_id = system_openfiles[id].fcb.d_index;
    // 加载这个磁盘i节点
    struct dinode* pdinode = (struct dinode*) malloc(sizeof(struct dinode));
    long addr = DINODESTART + dinode_id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread(pdinode, DINODESIZ, 1, disk);
    // 准备读取内容
    unsigned short size = pdinode->di_size;
    int block_num = size / BLOCKSIZ;
    char* res = (char*)malloc(size);
    for(i = 0; i < block_num; i++){
        addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread(res+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread(res+i*BLOCKSIZ, size-block_num*BLOCKSIZ, 1, disk);
    return res;
}

// 权限未实现
bool RunningSystem::writeFile(const char *pathname, int write_mode, std::string content){
    // 判断文件名是否合法
    if(!is_file(pathname)){
        return {};
    }
    // 判断用户对该文件是否有写权限
    // if(access())

    // 判断文件是否被用户打开
    // 获取用户的打开表
    user_open_table* userOpenTable = user_openfiles[cur_user];
    // 获取用户uid
    unsigned short p_uid = userOpenTable->p_uid;

    // 遍历查询该文件
    unsigned short id;
    bool found = false;
    int i;
    for(i = 0; i < NOFILE; i++){
        if(userOpenTable->items[i].f_inode == nullptr){
            continue;
        }
        id = userOpenTable->items[i].id_to_sysopen;
        if(!strcmp(system_openfiles[id].fcb.d_name, pathname) && system_openfiles[id].i_count != 0){
            found = true;
            break;
        }
    }
    // 用户没有打开该文件
    if(!found)
        return false;

    // 三种模式
}