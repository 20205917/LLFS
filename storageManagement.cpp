//
// Created by 86136 on 2023/5/25.
//
#include "RunningSystem.h"

//一个磁盘的i节点
static struct dinode block_buf[BLOCKSIZ / DINODESIZ];

// 从当前目录查找name对应的i节点
// 将会返回在数组中的下标，若为DIRNUM表明没找到
// 可能有问题
unsigned int namei(const string& name){
    // 从磁盘加载目录文件
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    auto* tmp = (struct dir*) malloc(sizeof(struct dir));
    unsigned int id;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DINODESTART + id * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char*)tmp+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DINODESTART + id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char*)tmp+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);

    // 开始查找
    bool found = false;

    for(i = 0; i < tmp->size; i++){
        if(!strcmp(tmp->files[i].d_name, name.c_str()) && tmp->files[i].d_index != 0){
            found = true;
            break;
        }
    }
    free(tmp);
    if(found)
        return i;
    else
        return DIRNUM;
}

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

// 释放磁盘块
// 成组链法的空间回收
// 要注意第一个组长块只有49块有效，其首位为0
void bfree(int block_num){
    if(block_num >= FILEBLK)
        return;
//    if(block_num == 1){
//        return;
//    }
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


//将数据区内容写回磁盘 内存中数据地址，硬盘索引数组，数据长度，文件指针
void write_data_back(void *data_address, const unsigned short *di_addr, int size, FILE *fp){
    int block_num = size / BLOCKSIZ;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        addr = DATASTART + di_addr[i] * BLOCKSIZ;
        fseek(fp, addr, SEEK_SET);
        fwrite((char*)data_address+i*BLOCKSIZ, BLOCKSIZ, 1, fp);
    }
    addr = DATASTART + di_addr[i] * BLOCKSIZ;
    fseek(fp, addr, SEEK_SET);
    fwrite((char*)data_address+i*BLOCKSIZ, size-block_num*BLOCKSIZ, 1, fp);
}

// 从硬盘读取一个硬盘i节点
inode* getDinodeFromDisk(unsigned int dinode_id){
    long addr = DINODESTART + dinode_id * DINODESIZ;
    auto* new_inode = (inode*)malloc(sizeof(struct inode));
    fseek(disk, addr, SEEK_SET);
    fread(&(new_inode->dinode.di_number), DINODESIZ, 1, disk);
    return new_inode;
}