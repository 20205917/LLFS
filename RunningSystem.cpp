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
    //
    // 初始化pwds
    // 初始化cur_path_inode
    cur_path_inode = (inode*)malloc(sizeof(struct inode));

};

RunningSystem::~RunningSystem(){
    fclose(disk);
}

