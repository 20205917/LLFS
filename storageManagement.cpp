//
// Created by 86136 on 2023/5/25.
//
#include "RunningSystem.h"

//一个磁盘的i节点
static struct dinode block_buf[BLOCKSIZ / DINODESIZ];

int ialloc(unsigned int) {
    unsigned int oneNum = BLOCKSIZ / DINODESIZ;

    if(file_system.s_pdinode == NICINOD){
        unsigned int count = 1;                     //已经扫描数
        unsigned int block_end_flag = 1;
        unsigned int cur_i = file_system.s_rdinode / oneNum;
        unsigned int cur_j = file_system.s_rdinode % oneNum;
        while (file_system.s_pdinode > 0 && count <= file_system.s_dinode_size){
            if (block_end_flag) {
                fseek(disk, DINODESTART + cur_i * DINODESIZ, SEEK_SET);
                fread(block_buf, 1, BLOCKSIZ,disk);
                block_end_flag = 0;
            }
            //略过非空闲块
            while (block_buf[cur_j].di_mode != DIEMPTY && cur_j < oneNum) {
                cur_j++;
                count++;
            }

            while (block_buf[cur_j].di_mode == DIEMPTY && cur_j < oneNum && file_system.s_pdinode > 0) {
                --file_system.s_pdinode;
                file_system.s_dinodes[file_system.s_pdinode] = cur_i*oneNum+cur_j;
                cur_j++;
                count++;
            }
            //满一块，扫下一块
            if(cur_j == oneNum) {
                block_end_flag = 1;
                cur_j = 0;
                cur_i++;
            }
        }


        file_system.s_rdinode = cur_i*oneNum+cur_j;
    }

    file_system.s_free_dinode_num--;
    file_system.s_fmod = SUPDATE;
    unsigned int result = file_system.s_dinodes[file_system.s_pdinode];
    file_system.s_dinodes[file_system.s_pdinode++] = 0;
    return result;
}

// 根据对应的硬盘i节点id从系统打中释放
void ifree(unsigned int dinode_id) {
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

//文件内容写回
void file_wirte_back(struct inode* inode){
    //TODO 多级索引
    // 需要抹除所占用的磁盘块
}

// 释放磁盘块
// 成组链法的空间回收
// 要注意第一个组长块只有49块有效，其首位为0
void bfree(int block_num){
    if(block_num == 1){
        return;
    }
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
unsigned int balloc(){
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