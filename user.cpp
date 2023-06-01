//
// Created by 86136 on 2023/6/1.
//

#include "RunningSystem.h"

int login(const string &pwd) {
    int i;
    //����Ƿ���ƥ���PWD
    for (i = 0; i < PWDNUM && strcmp(pwd.c_str(), pwds[i].password) != 0; i++);
    if (i == PWDNUM)
        return -1;

    //����Ƿ��Ѿ���¼
    if(!strcmp(pwd.c_str(), cur_user.c_str()))
        return -2;

    //
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        cur_user = pwd;
        return -3;
    }

    //���п�λ
    if (user_openfiles.size() < USERNUM) {
        auto *openTable = new user_open_table;
        openTable->p_uid = pwds[i].p_uid;
        openTable->p_gid = pwds[i].p_gid;
        memset(openTable->items, 0, sizeof(user_open_item) * NOFILE);
        user_openfiles.insert(pair<string, user_open_table *>(pwd, openTable));
        //���õ�ǰ�ļ�Ϊ��Ŀ¼
        cur_user = pwd;
        cur_dir_inode = iget(1);
        return 0;
    }
    return -3;
}

void logout(const string &pwd) {
    user_open_table *u = user_openfiles.find(pwd)->second;
    //�ر�ÿ���ļ�
    for (auto &item: u->items) {
        short id_to_sysopen = item.index_to_sysopen;
        if (id_to_sysopen != -1) {
            //�رպ����Ϊ����Ҫ�ͷ��ڴ�ڵ�
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
        return false;//û�ҵ����û�
    }
    user_open_table *T = user_openfiles.find(cur_user)->second;
    bool creator = file_inode->dinode.di_uid == T->p_uid;//�ļ���uid�����û���uid ˵���Ǵ�����
    bool group = file_inode->dinode.di_gid == T->p_gid;//�ļ���gid�����û���gid ˵�������ڳ�Ա
    if (file_inode->dinode.di_gid == 0 && operation == FDELETE)//��Ŀ¼����ɾ
        return false;
    if (creator || group && READ)
        return true;
    return false;
}


int switch_user(const string& pwd){
    int i;
    //����Ƿ���ƥ���PWD
    for (i = 0; i < PWDNUM && strcmp(pwd.c_str(), pwds[i].password) != 0; i++);
    if (i == PWDNUM)
        return -1; // pwd��Ч

    //����Ƿ��Ѿ���¼
    if(!strcmp(pwd.c_str(), cur_user.c_str()))
        return -2; // pwd����ǰ�û�

    //
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        cur_user = pwd;
        return 0; // �л��û��ɹ�
    }
    return -1;
}

// �����û�������
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
    return -1; // uid��Ч
}

// �����û�
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
        return -1; // �û��ﵽ����
    }
    pwds[index].p_uid = uid;
    pwds[index].p_gid = gid;
    strcpy(pwds[index].password, pwd.c_str());
    return 0; // ����û��ɹ�
}