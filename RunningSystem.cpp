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
    //
    // ��ʼ��pwds
    // ��ʼ��cur_path_inode
    cur_path_inode = (inode*)malloc(sizeof(struct inode));

};

RunningSystem::~RunningSystem(){
    fclose(disk);
}

