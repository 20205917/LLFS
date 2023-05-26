//
// Created by 86173 on 2023/5/25.
//
#include "config.h"
// 在内存i节点散列表根据硬盘i节点id查找
// 找不到就创建内存i节点
// 申请了额外空间
hinode iget(int dinode_id , hinode* &hinodes, FILE* disk){
    int inode_id = dinode_id % NHINO;
    if(hinodes[inode_id]->i_forw != NULL){
        hinode tmp = hinodes[inode_id]->i_forw;
        while(tmp){
            if(tmp->i_id == dinode_id){
                // 内存中已存在
                return tmp;
            }
            else
                tmp = tmp->i_forw;
        }
    }
    // 内存中不存在,需要创建
    long addr = DINODESTART + dinode_id * DINODESIZ;
    hinode newinode = (hinode)malloc(sizeof(struct inode));
    fseek(disk, addr, SEEK_SET);
    fread(&(newinode->dinode.di_number), DINODESIZ, 1, disk);
    // 加入内存散列表，作为hinodes[inode_id]的下一个节点
    newinode->i_forw = hinodes[inode_id]->i_forw;
    newinode->i_back = hinodes[inode_id];
    newinode->i_forw->i_back = newinode;
    hinodes[inode_id]->i_forw = newinode;
    // 补充初始化
    newinode->i_flag = 0;
    newinode->i_id = dinode_id;
    return newinode;
}

// 释放i节点回磁盘
// 如果被更改需要写回
// 如果被删除则在磁盘抹除
void iput(hinode inode, FILE* disk, struct super_block &file_system){
    if(inode->dinode.di_number != 0){
        // 需要写回
        if(inode->i_flag != 0){
            long addr = DINODESTART + inode->i_id * DINODESIZ;
            fseek(disk, addr, SEEK_SET);
            fwrite(&inode->dinode, DINODESIZ, 1, disk);
        }
    }
    else{
        // 需要抹除所占用的磁盘块
        int num = inode->dinode.di_size / BLOCKSIZ;
        for(int i = 0; i < num; i++){
            // 将对应磁盘块释放
            bfree(inode->dinode.di_addr[i], file_system, disk);
        }
        // 从系统打开表中释放
        ifree(inode->i_id, file_system);
    }
    // 从散列表中释放
    if(inode->i_forw == nullptr)
        inode->i_back->i_forw = nullptr;
    else{
        inode->i_forw->i_back = inode->i_back;
        inode->i_back->i_forw = inode->i_forw;
    }
    free(inode);
}

