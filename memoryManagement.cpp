//
// Created by 86173 on 2023/5/25.
//
#include "config.h"

//查看某个磁盘i节点id对应的内存i节点是否存在
hinode findHinode(int dinode_id , hinode* hinodes, FILE* disk){
    int inode_id = dinode_id % NHINO;
    if(hinodes[inode_id]->i_forw != NULL){
        hinode tmp = hinodes[inode_id]->i_forw;
        while(tmp){
            if(tmp->d_index == dinode_id){
                // 内存中已存在
                return tmp;
            }
            else
                tmp = tmp->i_forw;
        }  
    return NULL; 
}


// 在内存i节点散列表根据硬盘i节点id查找
// 找不到就创建内存i节点
// 申请了额外空间,//插入内存i节点  情况: 1-Hash缓冲区对应桶号已达可容纳i节点数上限7  2-未达上限，直接插入
hinode iget(int dinode_id , hinode* hinodes, FILE* disk){
    int inode_id = dinode_id % NHINO;
    hinode tmp=findHinode(dinode_id,hinodes,disk);
    if(tmp!=NULL)
        return tmp;
    // 内存中不存在,需要创建
    long addr = DINODESTART + dinode_id * DINODESIZ;
    hinode newinode = (hinode)malloc(sizeof(struct inode));
    fseek(disk, addr, SEEK_SET);
    fread(&(newinode->dinode.di_number), DINODESIZ, 1, disk);
    // 加入内存散列表，作为hinodes[inode_id]的下一个节点
    // if(hinodes[inode_id]->i_forw != NULL){
        
    //     newinode->i_forw = hinodes[inode_id]->i_forw;
    //     newinode->i_back = hinodes[inode_id];
    //     newinode->i_forw->i_back = newinode;
    //     hinodes[inode_id]->i_forw = newinode;
    // }
    // else{
    //     newinode->i_forw = hinodes[inode_id]->i_forw;
    //     newinode->i_back = hinodes[inode_id];
    // }
    hinode temp=hinodes[inode_id];
    for(int i=0;temp->i_forw!=NULL;i++){
        if(i==6)            
            iput(hinodes[inode_id],disk,file_system);
        temp=temp->i_forw;
    }
    temp->i_forw=newinode;
    newinode->i_forw=NULL;
    newinode->i_back=temp;

    // 补充初始化
    newinode->i_flag = 0;
    newinode->d_index = dinode_id;

    return newinode;
}

// 释放i节点回磁盘
// 如果被更改需要写回
// 如果被删除则在磁盘抹除(需要重构)
void iput(inode* inode, FILE* disk, struct super_block &file_system){
    if(inode->dinode.di_number != 0){
        // 需要写回
        if(inode->i_flag != 0){
            long addr = DINODESTART + inode->d_index * DINODESIZ;
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
        ifree(inode->d_index, file_system);
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

// 从当前目录查找name对应的i节点
// 将会返回在数组中的下标，若为DIRNUM表明没找到
// 可能有问题
unsigned int namei(char* name, hinode cur_path_inode, FILE* disk){
    // 从磁盘加载目录文件
    int size = cur_path_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    struct dir* tmp = (struct dir*) malloc(sizeof(struct dir));
    unsigned int id;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        id = cur_path_inode->dinode.di_addr[i];
        addr = DINODESTART + id * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char*)tmp+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_path_inode->dinode.di_addr[block_num];
    addr = DINODESTART + id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char*)tmp+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);

    // 开始查找
    bool found = false;

    for(i = 0; i < tmp->size; i++){
        if(!strcmp(tmp->files[i].d_name, name) && tmp->files[i].d_index != 0){
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
// 从当前目录找到下一个空的位置
// 将会返回在数组中的下标，若为-1表明没找到
int seek_catalog_leisure(inode* cur_path_inode, FILE* disk){//原来是iname
    // 从磁盘加载目录
    int size = cur_path_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    struct dir* tmp = (struct dir*) malloc(sizeof(struct dir));
    unsigned int id;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        id = cur_path_inode->dinode.di_addr[i];
        addr = DINODESTART + id * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char*)tmp+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_path_inode->dinode.di_addr[block_num];
    addr = DINODESTART + id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char*)tmp+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);

    // 开始查找
    bool found = false;

    for(i = 0; i < tmp->size; i++){
        if(tmp->files[i].d_index == 0){
            found = true;
            break;
        }
    }
    free(tmp);
    if(found){
        return i;
    }
    else
        return -1;
}
