//
// Created by 86136 on 2023/5/25.
//
#include "config.h"
#include "RunningSystem.h"

//一个磁盘的i节点
static struct dinode block_buf[BLOCKSIZ / DINODESIZ];

struct inode *ialloc(RunningSystem &runningSystem) {
    struct super_block &file_system = runningSystem.file_system;
    struct inode *temp_inode;

    unsigned int oneNum = BLOCKSIZ / DINODESIZ;

    if(file_system.s_pdinode == NICINOD){
        unsigned int count = 1;                     //已经扫描数
        unsigned int block_end_flag = 1;
        unsigned int cur_i = file_system.s_rdinode / oneNum;
        unsigned int cur_j = file_system.s_rdinode % oneNum;
        while (file_system.s_pdinode > 0 && count <= file_system.s_dinode_size){
            if (block_end_flag) {
                fseek(runningSystem.disk, DINODESTART + cur_i * DINODESIZ, SEEK_SET);
                fread(block_buf, 1, BLOCKSIZ,runningSystem.disk);
                block_end_flag = 0;
            }
            //找到空闲块
            while (block_buf[cur_j].di_mode == DIEMPTY && cur_j < oneNum) {
                cur_j++;
                count++;
            }
            //满一块，扫下一块
            if (cur_j == oneNum) {
                block_end_flag = 1;
                cur_j = 0;
                cur_i++;
            }
            else {
                file_system.s_dinodes[--file_system.s_pdinode] = cur_i*oneNum+cur_j;
                count++;
            }
        }
        file_system.s_rdinode = cur_i*oneNum+cur_j;
    }

    //申请内存i节点
    temp_inode = iget(file_system.s_dinodes[file_system.s_pdinode]);
    //写入初始化内容
    fseek(runningSystem.disk, DINODESTART + file_system.s_dinodes[file_system.s_pdinode] * DINODESIZ, SEEK_SET);
    fwrite(&temp_inode->dinode, 1, sizeof(struct dinode), runningSystem.disk);

    file_system.s_free_dinode_num--;
    file_system.s_pdinode++;
    file_system.s_fmod = SUPDATE;
    return temp_inode;
}


// 根据对应的硬盘i节点id从系统打开表中释放
// 然后判断该i节点是否应加入空闲i节点数组
void ifree(int dinode_id, struct super_block &file_system) {
    {
        //??????当空闲数组满时，应写回释放的dinode,防止下次搜索时类型没有覆盖。未满时不用写回，
    }
    file_system.s_free_dinode_num++;
    if (file_system.s_pdinode > 0 ) {
        // 未达到最大空闲块数
        file_system.s_dinodes[--file_system.s_pdinode] = dinode_id;
    }else {
        // 若到达，则判断铭记节点是否可以更换
        if (dinode_id < file_system.s_rdinode) {
            file_system.s_rdinode = dinode_id;
        }
    }
}

// 释放磁盘块
// 成组链法的空间回收
// 要注意第一个组长块只有49块有效，其首位为0
void bfree(int block_num, struct super_block &file_system, FILE* disk){
    if(file_system.s_pfree_block == NICFREE){
        // 栈满已经成组
        // 先写回磁盘
        fseek(disk, block_num * BLOCKSIZ, SEEK_SET);
        fwrite(file_system.s_free_blocks, 1, BLOCKSIZ, disk);
        // 然后视为栈空重新开始
        file_system.s_pfree_block = 1;
        file_system.s_free_blocks[0] = block_num;
        file_system.s_free_block_size++;
    }
    else{
        // 栈未满
        file_system.s_free_blocks[file_system.s_pfree_block++] = block_num;
        file_system.s_free_block_size++;
    }
}

// 分配磁盘块
unsigned int balloc(struct super_block &file_system, FILE *disk){
    if(file_system.s_free_block_size == 0){
        // 空间不足
        return DISKFULL;
    }
    unsigned int block_num = file_system.s_free_blocks[file_system.s_pfree_block-1];
    if(file_system.s_free_block_size == 1){
        // 若再分配则栈空，需要从磁盘加载组长块
        fseek(disk, block_num * BLOCKSIZ, SEEK_SET);
        fread(&file_system.s_free_blocks, BLOCKSIZ, 1, disk);
        file_system.s_free_block_size--;
        file_system.s_pfree_block = NICFREE;
    }
    else{
        file_system.s_free_block_size--;
        file_system.s_pfree_block--;
    }
    return block_num;
}