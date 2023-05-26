//
// Created by 86136 on 2023/5/25.
//

#include "RunningSystem.h"

RunningSystem::RunningSystem(){
    // 读硬盘
    disk = fopen("disk", "wb+");
    // 初始化file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // 初始化hinodes
    for(int i = 0; i < NHINO; i++){
        hinodes[i] == nullptr;
    }

    // 初始化system_openfiles
    for(int i = 0; i < SYSOPENFILE; i++){
        system_openfiles[i].i_count = 0;
        system_openfiles[i].fcb.d_ino = 0;
    }

    // 初始化pwds



    // 初始化cur_path_inode
    cur_path_inode = (inode*)malloc(sizeof(struct inode));

};

int  RunningSystem::login(char* pwd){
    int i,j;
    //检查是否有匹配的PWD
    for(i = 0; i  < PWDNUM &&strcmp(pwd,pwds[PWDNUM].password); i++);
    if(i==PWDNUM)
        return -1;

    //检查是否已经登录
    for(j = 0;j < USERNUM;j++){
        if(pwds[i].p_uid==user_openfiles[j].p_uid){
            return -2;
        }
    }

    //找到空位置
    for(j = 0;j < USERNUM;j++){
        if(0==user_openfiles[j].p_uid){
            //用户打开表初始化
            user_openfiles[j].p_uid = pwds[i].p_uid;
            user_openfiles[j].p_gid = pwds[i].p_gid;
            memset(user_openfiles[j].items,0,sizeof (user_open_item) * NOFILE);
            //设置当前文件为根目录

            //cur_path_inode = iget(1);

            return i;
        }
    }
    return -3;
}
void RunningSystem::logout(char* pwd){
    int i,j;
    //检查是否有匹配的PWD
    for(int i = 0; i  < PWDNUM &&strcmp(pwd,pwds[PWDNUM].password); i++);
    if(i==PWDNUM)
        return;

    //检查是否已经登录
    for(j = 0;j < USERNUM;j++){
        if(pwds[i].p_uid==user_openfiles[j].p_uid){
            //重置打开表
            user_openfiles[j].p_uid = 0;
            user_openfiles[j].p_gid = 0;
            //关闭所有打开文件
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

