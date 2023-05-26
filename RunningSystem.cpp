//
// Created by 86136 on 2023/5/25.
//

#include "RunningSystem.h"

RunningSystem::RunningSystem(){
    // ��Ӳ��
    disk = fopen("disk", "wb+");
    // ��ʼ��file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // ��ʼ��hinodes
    for(int i = 0; i < NHINO; i++){
        hinodes[i] == nullptr;
    }

    // ��ʼ��system_openfiles
    for(int i = 0; i < SYSOPENFILE; i++){
        system_openfiles[i].i_count = 0;
        system_openfiles[i].fcb.d_ino = 0;
    }

    // ��ʼ��pwds



    // ��ʼ��cur_path_inode
    cur_path_inode = (inode*)malloc(sizeof(struct inode));

};

int  RunningSystem::login(char* pwd){
    int i,j;
    //����Ƿ���ƥ���PWD
    for(i = 0; i  < PWDNUM &&strcmp(pwd,pwds[PWDNUM].password); i++);
    if(i==PWDNUM)
        return -1;

    //����Ƿ��Ѿ���¼
    for(j = 0;j < USERNUM;j++){
        if(pwds[i].p_uid==user_openfiles[j].p_uid){
            return -2;
        }
    }

    //�ҵ���λ��
    for(j = 0;j < USERNUM;j++){
        if(0==user_openfiles[j].p_uid){
            //�û��򿪱��ʼ��
            user_openfiles[j].p_uid = pwds[i].p_uid;
            user_openfiles[j].p_gid = pwds[i].p_gid;
            memset(user_openfiles[j].items,0,sizeof (user_open_item) * NOFILE);
            //���õ�ǰ�ļ�Ϊ��Ŀ¼

            //cur_path_inode = iget(1);

            return i;
        }
    }
    return -3;
}
void RunningSystem::logout(char* pwd){
    int i,j;
    //����Ƿ���ƥ���PWD
    for(int i = 0; i  < PWDNUM &&strcmp(pwd,pwds[PWDNUM].password); i++);
    if(i==PWDNUM)
        return;

    //����Ƿ��Ѿ���¼
    for(j = 0;j < USERNUM;j++){
        if(pwds[i].p_uid==user_openfiles[j].p_uid){
            //���ô򿪱�
            user_openfiles[j].p_uid = 0;
            user_openfiles[j].p_gid = 0;
            //�ر����д��ļ�
            for(int k = 0 ; k < NOFILE; k++){
                closeFile();
            }
            memset(user_openfiles[j].items,0,sizeof (user_open_item) * NOFILE);
            return;
        }
    }
    return;
}

RunningSystem::~RunningSystem(){
    fclose(disk);
}

