//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include "RunningSystem.h"

void initial(){
    // 读硬盘
    disk = fopen("disk", "rb+");
    char nothing[BLOCKSIZ] = {0};
    fseek(disk, 0, SEEK_SET);
    fwrite(nothing, BLOCKSIZ, FILEBLK + DINODEBLK + 2, disk);

    // 初始化超级块
    // 此时只有root对应的磁盘i节点和数据块
    // 占一个磁盘i节点和一个数据块，编号都为1
    file_system.s_block_size = FILEBLK - 1;
    file_system.s_dinode_size = BLOCKSIZ / DINODESIZ * DINODEBLK;
    file_system.s_free_dinode_num = file_system.s_dinode_size - 1;
    for(int i = 0; i < NICINOD; i++){
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = FILEBLK - 1;
    for(int i = FILEBLK; i > 1; i--){
        bfree(i);
    }
    file_system.s_fmod = '0';

    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);

    // 初始化root磁盘i节点
    cur_dir_inode = (hinode) malloc(sizeof(struct inode));
    cur_dir_inode->dinode.di_number = 0;
    cur_dir_inode->dinode.di_mode = DIDIR;
    cur_dir_inode->dinode.di_uid = 0;
    cur_dir_inode->dinode.di_gid = 0;
    cur_dir_inode->dinode.di_size = sizeof(struct dir);
    cur_dir_inode->dinode.di_addr[0] = 0;

    long addr = DINODESTART + 1 * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(cur_dir_inode->dinode), DINODESIZ, 1, disk);

    // 初始化root目录数据块内容
    root.size = 2;
    // 根目录
    root.files[0].d_index = 1;
    strcpy(root.files[0].d_name, "root");
    // 父目录
    root.files[1].d_index = 1;
    strcpy(root.files[1].d_name, "root");
    for(int i = 2; i < DIRNUM; i++){
        root.files[i].d_index = 0;
    }

    addr = DATASTART;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(root), sizeof(struct dir), 1, disk);

    // admin账户
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // 一个普通账户
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // 清空
    for(int i = 2; i < PWDNUM; i++){
        strcpy(pwds[0].password, "");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
    fclose(disk);
}

void install() {
    // 读硬盘
    disk = fopen("disk", "rb+");

    // 初始化file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // 初始化hinodes
    for (auto &i: hinodes) {
        i = (hinode) malloc(sizeof(struct inode));
        memset(i,0,sizeof (inode));
    }

    // 初始化system_openfiles
    for (auto &system_openfile: system_openfiles) {
        system_openfile.i_count = 0;
        system_openfile.fcb.d_index = 0;
    }

    // 初始化pwds
    // 其实是读第一个块
    fseek(disk, 0, SEEK_SET);
    fread(&pwds, sizeof(PWD), PWDNUM, disk);

    // 初始化user_openfiles
    user_openfiles.clear();


    // 读取root目录和cur_dir当前目录
    // 即第一个i节点
    // 初始化cur_dir_inode
    cur_dir_inode = iget(1);
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for (i = 0; i < block_num; i++) {
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DATASTART + id * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&root) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&cur_dir) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&root) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&cur_dir) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);

}

// 格式化
void format(){
    // admin账户
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // 一个普通账户
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // 清空
    for(int i = 2; i < PWDNUM; i++){
        strcpy(pwds[0].password, "");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);

    // 重置超级块
    // 此时只有root对应的磁盘i节点和数据块
    // 占一个磁盘i节点和一个数据块，编号都为1
    file_system.s_block_size = FILEBLK - 1;
    file_system.s_dinode_size = BLOCKSIZ / DINODESIZ * DINODEBLK;
    file_system.s_free_dinode_num = file_system.s_dinode_size - 1;
    for(int i = 0; i < NICINOD; i++){
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = FILEBLK - 1;
    for(int i = FILEBLK; i > 1; i--){
        bfree(i);
    }
    file_system.s_fmod = '0';

    // 清空系统打开表
    for(auto & system_openfile : system_openfiles){
        system_openfile.i_count = 0;
        system_openfile.fcb.d_index = 0;
    }

    // 清空用户打开表
    user_openfiles.clear();

    // 清空内存结点缓存
    for(int i = 0; i < NHINO; i++){
        while(hinodes[i]->i_forw){
            iput(hinodes[i]->i_forw);
        }
    }

    // 当前目录置为root
    cur_dir_inode = iget(1);
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for (i = 0; i < block_num; i++) {
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DATASTART + id * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&cur_dir) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&cur_dir) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);
}

// 关机并保存
void halt(){
    // 检查当前用户表，释放
    for(const auto& data: user_openfiles){
        user_open_table* tmp = data.second;
        for(int i = 0; i < NOFILE; i++){
            if(tmp->items[i].f_inode == nullptr)
                continue;
            int id = tmp->items[i].index_to_sysopen;
            system_openfiles[id].i_count--;
            if(system_openfiles[id].i_count == 0)
                system_openfiles[id].fcb.d_index = 0;
        }
    }
    user_openfiles.clear();

    // 清空缓存
    for(int i = 0; i < NHINO; i++){
        while(hinodes[i]->i_forw){
            iput(hinodes[i]->i_forw);
        }
    }

    // 保存超级块
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);
    // 关闭磁盘
    fclose(disk);
}

int login(const string &pwd) {
    int i;
    //检查是否有匹配的PWD
    for (i = 0; i < PWDNUM && strcmp(pwd.c_str(), pwds[i].password) != 0; i++);
    if (i == PWDNUM)
        return -1;

    //检查是否已经登录
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        return -2;
    }

    //还有空位
    if (user_openfiles.size() < USERNUM) {
        auto *openTable = new user_open_table;
        openTable->p_uid = pwds[i].p_uid;
        openTable->p_gid = pwds[i].p_gid;
        memset(openTable->items, 0, sizeof(user_open_item) * NOFILE);
        user_openfiles.insert(pair<string, user_open_table *>(pwd, openTable));
        //设置当前文件为根目录
        cur_user = pwd;
        cur_dir_inode = iget(1);
        cur_dir = get_dir(1);
        return 0;
    }
    return -3;
}

void logout(const string &pwd) {
    user_open_table *u = user_openfiles.find(pwd)->second;
    //关闭每个文件
    for (auto &item: u->items) {
        unsigned short id_to_sysopen = item.index_to_sysopen;
        if ( id_to_sysopen!= -1) {
            //关闭后打开数为零需要释放内存节点
            system_openfiles[id_to_sysopen].i_count--;
            if(system_openfiles[id_to_sysopen].i_count == 0){
                iput(iget(system_openfiles[id_to_sysopen].fcb.d_index));
            }
        }
    }
    free(u);
    user_openfiles.erase(pwd);
    cur_user = "";
}

bool access(int operation, inode *file_inode) {
    if (user_openfiles.find(cur_user) == user_openfiles.end()) {
        return false;//没找到该用户
    }
    user_open_table *T = user_openfiles.find(cur_user)->second;
    bool creat = file_inode->dinode.di_uid == T->p_uid;//文件的uid等于用户的uid 说明是创建者
    bool group = file_inode->dinode.di_gid == T->p_gid;//文件的gid等于用户的gid 说明是组内成员
    if(creat || group && READ)
        return true;
    return false;
}

//判断基础合法性，存在，长度，位于根目录
//清理多余合法符号
//具体待修改.(暂定）


/* 目录文件数据区是否有空闲，是否在父目录数据区里有已存在文件(findfile()实现)，有空闲
 申请磁盘i结点，初始化磁盘i结点，磁盘i结点写回磁盘，父目录数据区写回磁盘。
 申请磁盘i节点未实现*/
//inode* createFile(string pathname, unsigned short di_mode){
//    if(cur_user == "")
//        return nullptr;
//
//    FCB  fcb;
//    int i, j;
//    //查找文件
//    fcb = addFile(pathname);
//    //文件已经存在
//    if (fcb.d_index != 0)
//        return NULL;
//    else
//    {
//        struct inode* inode = ialloc(*this);
//        //在目录中加入
//        ;
//        inode->dinode.di_mode = 1;
//        inode->dinode.di_uid = user_openfiles.find(cur_user)->second->p_uid;
//        inode->dinode.di_gid = user_openfiles.find(cur_user)->second->p_gid;
//        inode->dinode.di_size = 0;
//        inode->ifChange = 1;                //需要重新写回
//        return j;
//    }
//}

//int openFile(const string& pathname, unsigned short flags) {
//    //判断合法性
//    string path = clean_path(pathname);
//    //clean_path清除
//    if (path.empty())
//        return -1;
//    //寻找文件
//    inode* new_inode = find_file(path);
//    if(fcb.d_index == 0){
//        return -2;}
//
//    //磁盘节点读入内存
//    inode* inode = iget(fcb.d_index,hinodes,disk);
//
//    //存在则查找访问权限
//    if(access(R, inode)){
//        return -3;
//    }
//
//    //放入系统打开表
//    int index_to_system;
//    for (index_to_system = 0; index_to_system < SYSOPENFILE; index_to_system++)
//        if (system_openfiles[index_to_system].i_count == 0) break;
//    if (index_to_system == SYSOPENFILE) {
//        iput(inode);
//        return -4;
//    }
//    system_openfiles[index_to_system].i_count = 1;
//    system_openfiles->fcb = fcb;
//
//    //放入用户打开表
//    user_open_table* u = user_openfiles.find(cur_user)->second;
//    int fd;
//    for (int fd = 0;fd < NOFILE;fd++){
//        if(u->items[fd].f_count==0){
//            u->items[fd].f_count = 1;
//            u->items[fd].u_default_mode = flags;
//            u->items[fd].f_offset = 0;
//            u->items[fd].index_to_sysopen = index_to_system;
//            u->items[fd].f_inode = inode;
//            break;
//        }
//    };
//
//    /*if APPEND, free the block of the file before */
//    if (flags & FAPPEND) {
//        for (index_to_system = 0; index_to_system < inode->di_size / BLOCKSIZ + 1; index_to_system++)
//            bfree(inode->di_addr[index_to_system]);
//        inode->di_size = 0;
//    }
//
//    return fd;
//}

struct dir get_dir(unsigned int d_index) {
    inode *dir_inode = iget(d_index);
    // 从磁盘加载目录
    struct dir work_dir{};
    work_dir.size = dir_inode->dinode.di_size / DIRSIZ ;
    int i = 0;
    for (i = 0; i < work_dir.size / (BLOCKSIZ / DIRSIZ ); i++) {
        fseek(disk, DATASTART + BLOCKSIZ * dir_inode->dinode.di_addr[i], SEEK_SET);
        fread(&work_dir.files[BLOCKSIZ / DIRSIZ * i], 1, BLOCKSIZ, disk);
    }
    fseek(disk, DATASTART + BLOCKSIZ * dir_inode->dinode.di_addr[i], SEEK_SET);
    fread(&work_dir.files[BLOCKSIZ / DIRSIZ * i], 1, dir_inode->dinode.di_size % BLOCKSIZ, disk);
    return work_dir;
}
int mkdir(string &pathname) {
    if (!judge_path(pathname)) {
        return false;
    } else {
        inode *catalog;
        struct dir catalog_dir;
        string filename;
        if(pathname.find_last_of('/')==std::string::npos){
            catalog = cur_dir_inode;
            catalog_dir = cur_dir;
            filename = pathname;
        }
        else{
            int pos = pathname.find_last_of('/') + 1;
            string father_path = pathname.substr(0, pos - 1);
            filename = pathname.substr(pos);
            catalog = find_file(father_path);
            if (catalog == nullptr) {
                return -1;//无该路径，返回错误码
            }
            catalog_dir = get_dir(catalog->d_index);
        }
        if (access(CHANGE, catalog))
            return -1;//权限不足，返回错误码
        //判断目录文件数据区是否有空闲
        //小于最大目录数，说明空闲
        if (catalog_dir.size < DIRNUM) {
            //判断是否重复
            for (auto & i : catalog_dir.files) {
                if (strcmp(i.d_name,filename.data())) //如果有已经存在的文件夹，则返回错误码
                    return -1;
            }
            //申请索引结点和硬盘数据区
            int new_d_index = ialloc(1);
            inode *new_inode = iget(new_d_index);
            int block_amount = sizeof(dir) / BLOCKSIZ + 1;
            for (int j = 0; j < block_amount; j++) {
                new_inode->dinode.di_addr[j] = balloc();
            }
            new_inode->dinode.di_mode = DIDIR;
            //初始化硬盘数据区(索引结点区在ialloc中初始化)
            struct dir new_dir = get_dir(new_d_index);
            char* tmp = "root";
            strcpy(new_dir.files[0].d_name,tmp);
            new_dir.files[0].d_index = 1;
            new_dir.size = 0;
            //找到父目录空闲的目录项,写入文件名和文件磁盘结点
            int leisure = seek_catalog_leisure();
            strcpy(catalog_dir.files[leisure].d_name,filename.data());
            catalog_dir.files[leisure].d_index = new_d_index;
            catalog_dir.size++;
            catalog->ifChange = 1;
            new_inode->ifChange = 1;
            //将父目录的内存i结点写入磁盘i结点，将新文件夹的内存i结点写入磁盘i结点
            iput(catalog);
            iput(new_inode);
            return 1;
        } else {//没有空闲，失败
            return -1;
        }
    }
}


int rmdir(const string& pathname) {
    if (!judge_path(pathname)) {
        return -1;
    } else {
        inode *father_catalog;
        string filename;
        if(pathname.find_last_of('/')==std::string::npos){
            father_catalog = cur_dir_inode;
            filename = pathname;
        }
        else{
            int pos = pathname.find_last_of('/') + 1;
            string father_path = pathname.substr(0, pos - 1);
            father_catalog = find_file(father_path);
            if(father_catalog == nullptr)
                return -1;//无该路径，返回错误码
            filename = pathname.substr(pos);
        }
        inode *catalog = find_file(pathname);
        if (access(CHANGE, father_catalog))
            return -1;//权限不足，返回错误码
        if (catalog == nullptr) {
            return -1;//无该路径，返回错误码
        }
        struct dir catalog_dir = get_dir(catalog->d_index);//得到路径的目录dir数据
        struct dir father_dir = get_dir(father_catalog->d_index);//得到父目录的dir数据
        if (catalog_dir.size != 0) {
            return -1;//该路径的目录有内容，失败。
        } else {
            //将父目录里该项内容删除
            for (auto &i: father_dir.files) {
                if (i.d_index == catalog->d_index) {//查找该文件的下标
                    i.d_index = 0;
                    memset(i.d_name, 0, DIRSIZ);
                    father_dir.size--;
                    catalog->ifChange = 1;
                    father_catalog->ifChange = 1;
                    catalog->dinode.di_number--;
                    break;
                }
            }
            //将子目录磁盘i结点删除，释放该结点所指的数据区
            iput(catalog);
            //将父目录数据写回磁盘数据区
            iput(father_catalog);
            return 1;
        }
    }
}

//移动系统当前路径
int chdir(const string& pathname) {
    if (!judge_path(pathname)) {
        return -1;
    } else {
        inode *catalog = find_file(pathname);
        if (access(CHANGE, catalog))
            return -1;//权限不足，返回错误码
        if (catalog == nullptr) {
            return -1;//无该路径，返回错误码
        }
        cur_dir = get_dir(catalog->d_index);
        cur_dir_inode = catalog;
    }
    return 1;
}

int show_dir() {
    if (access(READ, cur_dir_inode))
        return -1;//权限不足，返回错误码
    for (auto & file : cur_dir.files) {
        if (file.d_index != 0) {
            cout << file.d_name << endl;//输出当前路径下的文件内容
        }
    }
    return 0;
}

//显示当前用户ss
string whoami() {
    return cur_user;
}


/* 文件名是否合法，需要findfile，判断是否有权限
// 修改父目录数据区并写入磁盘，iput()删除文件
// false删除失败 true删除成功
// 权限未实现 iput未实现*/
bool deleteFile(const string& pathname) {
    // 判断文件名是否合法
    if (!is_file(pathname)) {
        return false;
    }
    hinode res_inode = find_file(pathname);
    // 是否在父目录数据区里有该文件
    if (res_inode->d_index == 0) {
        return false;
    }
    // 判断用户对父目录有写权限和执行权限是否
    // if(access())

    // 修改父目录数据区
    // 更改目录项
    unsigned int index = namei(pathname);
    cur_dir.files[index].d_index = 0;
    cur_dir.size--;
    // 写入磁盘
    file_wirte_back(cur_dir_inode);
    // iput()删除文件
    // iput(res_inode);

    return true;
}

// 关闭一个已经被用户打开了的文件

void closeFile(const string& pathname) {
    // 判断文件名是否合法
    if (!is_file(pathname)) {
        return;
    }
    // 获取用户的打开表
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // 获取用户uid


    // 遍历查询该文件
    unsigned short id;
    bool found = false;
    int i;
    for (i = 0; i < NOFILE; i++) {
        if (userOpenTable->items[i].f_inode == nullptr) {
            continue;
        }
        id = userOpenTable->items[i].index_to_sysopen;
        if (!strcmp(system_openfiles[id].fcb.d_name, pathname.c_str()) && system_openfiles[id].i_count != 0) {
            found = true;
            break;
        }
    }
    // 用户没有打开该文件
    if (!found)
        return;

    // 确定打开了
    // 释放该文件的内存i节点
    iput(userOpenTable->items[i].f_inode);
    userOpenTable->items[i].f_inode = nullptr;
    // 系统打开表判断是否需要关闭
    system_openfiles[id].i_count--;
    if (system_openfiles[id].i_count == 0) {
        // 将这个文件从系统打开表中关闭
        system_openfiles[id].fcb.d_index = 0;
    }
}

// 从用户以打开的文件中读取内容
// 以字符形式返回内容
// 没有实现权限判断
string readFile(string pathname) {
    // 判断文件名是否合法
    if (!is_file(pathname)) {
        return {};
    }

    // 判断用户对该文件是否有读权限
    // if(access())

    // 判断文件是否被用户打开
    // 获取用户的打开表
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // 获取用户uid

    // 遍历查询该文件
    unsigned short id;
    bool found = false;
    int i;
    for (i = 0; i < NOFILE; i++) {
        if (userOpenTable->items[i].f_inode == nullptr) {
            continue;
        }
        id = userOpenTable->items[i].index_to_sysopen;
        if (!strcmp(system_openfiles[id].fcb.d_name, pathname.c_str()) && system_openfiles[id].i_count != 0) {
            found = true;
            break;
        }
    }
    // 用户没有打开该文件
    if (!found)
        return {};

    // 确定打开了
    unsigned int dinode_id = system_openfiles[id].fcb.d_index;
    // 加载这个磁盘i节点
    auto *pdinode = (struct dinode *) malloc(sizeof(struct dinode));
    long addr = DINODESTART + dinode_id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread(pdinode, DINODESIZ, 1, disk);
    // 准备读取内容
    unsigned short size = pdinode->di_size;
    int block_num = size / BLOCKSIZ;
    char *res = (char *) malloc(size);
    for (i = 0; i < block_num; i++) {
        addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread(res + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread(res + i * BLOCKSIZ, size - block_num * BLOCKSIZ, 1, disk);
    return res;
}

inode *find_file(string addr) {
    string Subaddr;
    unsigned int index;//保存根据文件名找到的FCB的d_index
    int isInDir = 0;//判断是否在目录中
    int first;//第一次出现'/'的位置
    if (addr.empty())
        return nullptr;
    if (addr[addr.length() - 1] == '/')
        return nullptr;
    if (addr[0] == '/') {//绝对路径
        addr = addr.substr(1, addr.length());
        index = cur_dir.files[0].d_index;
        cur_dir = get_dir(index);
    }

    //依次查找各个目录内的下一级目录
    while (addr.find_first_of('/') != string::npos) {
        first = addr.find_first_of('/');
        Subaddr = addr.substr(0, first);
        addr = addr.substr(first + 1, addr.length());
        for (auto &file: cur_dir.files) {
            if (strcmp(file.d_name, Subaddr.c_str()) == 0) {
                index = file.d_index;
                cur_dir = get_dir(index);
                isInDir = 1;
                break;
            }
        }
        if (isInDir == 0)
            return nullptr;
        isInDir = 0;
    }

    //得到最终文件的内存i结点指针
    for (auto &file: cur_dir.files) {
        if (strcmp(file.d_name, addr.c_str()) == 0)
            return iget(file.d_index);
    }
    return nullptr;

}

// 权限未实现
//bool writeFile(const string& pathname, int write_mode,const string& content) {
//    // 判断文件名是否合法
//    if (!is_file(pathname)) {
//        return {};
//    }
    // 判断用户对该文件是否有写权限
    // if(access())

    // 判断文件是否被用户打开
    // 获取用户的打开表
//    user_open_table *userOpenTable = user_openfiles[cur_user];
//    // 获取用户uid
//    unsigned short p_uid = userOpenTable->p_uid;
//
//    // 遍历查询该文件
//    unsigned short id;
//    bool found = false;
//    int i;
//    for (i = 0; i < NOFILE; i++) {
//        if (userOpenTable->items[i].f_inode == nullptr) {
//            continue;
//        }
//        id = userOpenTable->items[i].index_to_sysopen;
//        if (!strcmp(system_openfiles[id].fcb.d_name, pathname.c_str()) && system_openfiles[id].i_count != 0) {
//            found = true;
//            break;
//        }
//    }
//    // 用户没有打开该文件
//    if (!found)
//        return false;
//
//    // 三种模式
//}