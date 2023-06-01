//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include <iomanip>
#include <algorithm>
#include "RunningSystem.h"

void initial() {
    // 读硬盘
    disk = fopen(disk_file_name, "wb+");
    char nothing[BLOCKSIZ] = {0};
    fseek(disk, 0, SEEK_SET);
    for(int i = 0; i < FILEBLK + DINODEBLK + 2; i++)
        fwrite(nothing, BLOCKSIZ, 1, disk);

    // 初始化超级块
    // 此时只有root对应的磁盘i节点和数据块
    // 占一个磁盘i节点和一个数据块，编号都为1
    file_system.s_block_size = FILEBLK - 1;
    file_system.s_dinode_size = BLOCKSIZ / DINODESIZ * DINODEBLK;
    file_system.s_free_dinode_num = file_system.s_dinode_size - 1;
    for (int i = 0; i < NICINOD; i++) {
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = 0;
    file_system.s_pfree_block = 0;
    for (int i = FILEBLK; i > 1; i--) {
        bfree(i);
    }
    file_system.s_fmod = '0';

    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);

    // 初始化root磁盘i节点
    cur_dir_inode = (hinode) malloc(sizeof(struct inode));
    cur_dir_inode->dinode.di_number = 1;
    cur_dir_inode->dinode.di_mode = DIDIR;
    cur_dir_inode->dinode.di_uid = 0;
    cur_dir_inode->dinode.di_gid = 0;
    cur_dir_inode->dinode.di_size = sizeof(struct dir);
    cur_dir_inode->dinode.di_addr[0] = 0;

    long addr = DINODESTART + 1 * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(cur_dir_inode->dinode), DINODESIZ, 1, disk);

    struct dir root;
    // 初始化root目录数据块内容
    root.size = 2;
    // 根目录
    root.files[0].d_index = 1;
    strcpy(root.files[0].d_name, "root");
    // 父目录
    root.files[1].d_index = 1;
    strcpy(root.files[1].d_name, "root");
    for (int i = 2; i < DIRNUM; i++) {
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
    for (int i = 2; i < PWDNUM; i++) {
        pwds[i].p_uid = 0;
        memset(pwds[i].password, 0, 12);
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
    fclose(disk);
}

void install() {
    // 读硬盘
    disk = fopen(disk_file_name, "rb+");

    // 初始化file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // 初始化hinodes
    for (auto &i: hinodes) {
        i = (hinode) malloc(sizeof(struct inode));
        memset(i, 0, sizeof(inode));
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
    cur_path = "root";
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for (i = 0; i < block_num; i++) {
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DATASTART + id * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&cur_dir_inode->content) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    cur_dir_inode->content = malloc(size - BLOCKSIZ * block_num);
    fread((char *) (cur_dir_inode->content) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);

}

// 格式化
void format() {
    // admin账户
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // 一个普通账户
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // 清空
    for (int i = 2; i < PWDNUM; i++) {
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
    for (int i = 0; i < NICINOD; i++) {
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = 0;
    file_system.s_pfree_block = 0;
    for (int i = FILEBLK; i > 1; i--) {
        bfree(i);
    }
    file_system.s_fmod = '0';

    // 清空系统打开表
    for (auto &system_openfile: system_openfiles) {
        system_openfile.i_count = 0;
        system_openfile.fcb.d_index = 0;
    }

    // 清空用户打开表
    user_openfiles.clear();

    // 清空内存结点缓存
    for (auto & hinode : hinodes) {
        while (hinode->i_forw) {
            iput(hinode->i_forw);
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
        fread((char *) (&cur_dir_inode->content) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&cur_dir_inode->content) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);
    // admin账户
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // 一个普通账户
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // 清空
    for (int j = 2; j < PWDNUM; j++) {
        pwds[j].p_uid = 0;
        memset(pwds[j].password, 0, 12);
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
}

// 关机并保存
void halt() {
    // 检查当前用户表，释放
    for (const auto &data: user_openfiles) {
        user_open_table *tmp = data.second;
        for (auto & item : tmp->items) {
            if (item.f_inode == nullptr)
                continue;
            int id = item.index_to_sysopen;
            system_openfiles[id].i_count--;
            if (system_openfiles[id].i_count == 0)
                system_openfiles[id].fcb.d_index = 0;
        }
    }
    user_openfiles.clear();

    // 清空缓存
    for (auto & hinode : hinodes) {
        while (hinode->i_forw) {
            iput(hinode->i_forw);
        }
    }

    // 保存超级块
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);

    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
    // 关闭磁盘
    fclose(disk);
}



//显示当前用户ss
string whoami() {
    return cur_user;
}


// 展示所有用户
void show_all_users(){
    std::cout << "uid" << "     gid" << "     pwd" << std::endl;
    for(int i = 0; i < USERNUM; i++){
        if(pwds[i].password[0] != '\0'){
            std::cout << pwds[i].p_uid << "       "
            << pwds[i].p_gid << "       "
            << pwds[i].password << std::endl;
        }
    }
}

// 展示所有登录用户
void show_login_users(){
    std::cout << "uid" << "     gid" << "     pwd" << std::endl;
    for(const auto& user: user_openfiles){
        for(int i = 0; i < USERNUM; i++){
            if(!strcmp(pwds[i].password, user.first.c_str())){
                std::cout << pwds[i].p_uid << "       "
                          << pwds[i].p_gid << "       "
                          << pwds[i].password << std::endl;
            }
        }
    }
}



// 显示当前用户打开的文件信息
void show_user_opened_files(){
    auto items = user_openfiles.find(cur_user)->second->items;
    std::cout << "filename" << "          fd" << "   count"<< "    offset" << std::endl;
    for(int i = 0; i < NOFILE; i++){
        if(items[i].f_count != 0)
            std::cout << setiosflags(ios::left)  << setw(17) << system_openfiles[items[i].index_to_sysopen].fcb.d_name
                      << items[i].index_to_sysopen
                      << "     " << items[i].f_count
                      << "        " << items[i].f_offset
                      << std::endl;
    }
}
// 显示所有用户打开的文件信息
void show_opened_files(){

    std::cout <<"uid" << "    filename" << "         fd" << "    count" << std::endl;
    for(const auto& user_openfile: user_openfiles){
        if(user_openfile.second == nullptr)
            continue;
        auto items = user_openfile.second->items;
        for(int i = 0; i < NOFILE; i++){
            if(items[i].f_count != 0)
                std::cout << setiosflags(ios::left)  << user_openfile.second->p_uid
                          << "      "<< setw(17) << system_openfiles[items[i].index_to_sysopen].fcb.d_name
                          << " " << items[i].index_to_sysopen
                          << "    " << items[i].f_count
                          << std::endl;
        }
    }
}
// 显示系统打开表
void show_sys_opened_files(){
    std::cout << "filename" << "         d_index" << "    count" << std::endl;
    for(auto & system_openfile : system_openfiles){
        if(system_openfile.i_count != 0){
            std::cout << setiosflags(ios::left) << setw(17) << system_openfile.fcb.d_name
                      << system_openfile.fcb.d_index
                      << "          " << system_openfile.i_count
                      << std::endl;
        }
    }
}

//路径是否合法
//目录是否合法  0- 错误输入 1-目录 2-非目录文件
int judge_path(string pathname) {
    string Subpathname;
    int first;//第一次出现'/'的位置
    if (pathname.empty() || (pathname[pathname.length() - 1] == '/'&&pathname.length()!=1))        //最后一个字符为'/'以及string为空时错
        return 0;
    if (pathname[0] == '/')//绝对路径
        pathname = pathname.substr(1, pathname.length());      //除去第一个字符'/'
    //依次顺地址往下一个目录，判断文件名是否合法以及是否出现连续的'/'
    while (pathname.find_first_of('/') != string::npos) {
        if (pathname[0] == '/')        //说明出现了连续的'/'
            return 0;
        first = pathname.find_first_of('/');
        Subpathname = pathname.substr(0, first);
        pathname = pathname.substr(first + 1, pathname.length());
        if (Subpathname.find('.') != string::npos)     //目录名中出现'.'
            return 0;
    }
    //走出上面循环后的pathname为最后一级的文件名
    if (pathname.find('.') == string::npos)        //说明不含'.'，为目录名
        return 1;

    int count = std::count(pathname.begin(), pathname.end(), '.');
    if (count > 1)         //文件名中不可以含有一个以上的'.'
        return 0;
    return 2;
}