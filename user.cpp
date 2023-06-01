//
// Created by 86136 on 2023/6/1.
//

#include "RunningSystem.h"

int login(const string &pwd) {
    int i;
    //检查是否有匹配的PWD
    for (i = 0; i < PWDNUM && strcmp(pwd.c_str(), pwds[i].password) != 0; i++);
    if (i == PWDNUM)
        return -1;

    //检查是否已经登录
    if(!strcmp(pwd.c_str(), cur_user.c_str()))
        return -2;

    //
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        cur_user = pwd;
        return -3;
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
        return 0;
    }
    return -3;
}

void logout(const string &pwd) {
    user_open_table *u = user_openfiles.find(pwd)->second;
    //关闭每个文件
    for (auto &item: u->items) {
        short id_to_sysopen = item.index_to_sysopen;
        if (id_to_sysopen != -1) {
            //关闭后打开数为零需要释放内存节点
            system_openfiles[id_to_sysopen].i_count--;
            if (system_openfiles[id_to_sysopen].i_count == 0) {
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
    bool creator = file_inode->dinode.di_uid == T->p_uid;//文件的uid等于用户的uid 说明是创建者
    bool group = file_inode->dinode.di_gid == T->p_gid;//文件的gid等于用户的gid 说明是组内成员
    if (file_inode->dinode.di_gid == 0 && operation == FDELETE)//根目录不能删
        return false;
    if (creator || group && READ)
        return true;
    return false;
}


int switch_user(const string& pwd){
    int i;
    //检查是否有匹配的PWD
    for (i = 0; i < PWDNUM && strcmp(pwd.c_str(), pwds[i].password) != 0; i++);
    if (i == PWDNUM)
        return -1; // pwd无效

    //检查是否已经登录
    if(!strcmp(pwd.c_str(), cur_user.c_str()))
        return -2; // pwd即当前用户

    //
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        cur_user = pwd;
        return 0; // 切换用户成功
    }
    return -1;
}

// 更改用户所在组
int usermod(int uid, int gid){
    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, "")){
            continue;
        }
        if(uid == pwds[i].p_uid){
            pwds[i].p_gid = gid;
            return 0;
        }
    }
    return -1; // uid无效
}

// 创建用户
int useradd(int gid, const std::string& pwd){
    int uid = 0;
    int index = 0;
    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, "")){
            index = i;
            continue;
        }
        if(uid <= pwds[i].p_uid){
            uid = pwds[i].p_uid + 1;
        }
    }
    if(index == 0){
        return -1; // 用户达到上限
    }
    pwds[index].p_uid = uid;
    pwds[index].p_gid = gid;
    strcpy(pwds[index].password, pwd.c_str());
    return 0; // 添加用户成功
}