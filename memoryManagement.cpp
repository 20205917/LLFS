//
// Created by 86173 on 2023/5/25.
//
#include "config.h"

//查看某个磁盘i节点id对应的内存i节点是否存在
inode* findHinode(int dinode_id , hinode* hinodes, FILE* disk){
    int inode_id = dinode_id % NHINO;
    if(hinodes[inode_id]->i_forw != NULL) {
        hinode tmp = hinodes[inode_id]->i_forw;
        while (tmp) {
            if (tmp->d_index == dinode_id) {
                // 内存中已存在
                return tmp;
            } else
                tmp = tmp->i_forw;
        }
    }
    return NULL; 
}


// 在内存i节点散列表根据硬盘i节点id查找
// 找不到就创建内存i节点
// 申请了额外空间,//插入内存i节点  情况: 1-Hash缓冲区对应桶号已达可容纳i节点数上限7  2-未达上限，直接插入
inode* iget(int dinode_id , hinode* hinodes, FILE* disk){

    hinode tmp= findHinode(dinode_id, hinodes, disk);
    if(tmp!=NULL)
        return tmp;
    // 内存中不存在,需要创建
    long addr = DINODESTART + dinode_id * DINODESIZ;
    inode* newinode = (hinode)malloc(sizeof(struct inode));
    fseek(disk, addr, SEEK_SET);
    fread(&(newinode->dinode), DINODESIZ, 1, disk);
    int inode_id = dinode_id%128;

    hinode temp=hinodes[inode_id]->i_back;


    //TODO remake
    if(temp!=NULL){
        if(temp->s_num==7)
            iput(hinodes[inode_id]->i_forw,disk,file_system);
        temp->form=newinode;
        newinode->i_back=temp;
        newinode->i_forw=hinodes[inode_id];
        hinodes[inode_id]->i_back=newinode;
        newinode->s_num=temp->s_num+1;
    }
    else{
        newinode->i_back=hinodes[inode_id];
        newinode->i_forw=hinodes[inode_id];
        hinodes[inode_id]->i_forw=newinode;
        hinodes[inode_id]->i_back=newinode;
        newinode->s_num=1;
    }


    // 补充初始化
    newinode->i_flag = 0;
    newinode->ifChange = 0;
    newinode->d_index = dinode_id;
    return newinode;
}

// 释放i节点回磁盘
// 如果被更改需要写回
// 如果硬连接数为0
bool iput(inode* inode){
    if(inode->dinode.di_number == 0){
        if(inode->dinode.di_size>0){
            //TODO 多级索引释放数据区
        }
        //修改文件类型为空
        inode->ifChange = 1;
        inode->dinode.di_mode = ENOTEMPTY;

        // 释放磁盘i节点，只释放点
        ifree(inode->d_index);
    }

    if(inode->ifChange != 0){
        long addr = DINODESTART + inode->d_index * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fwrite(&inode->dinode, DINODESIZ, 1, disk);
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
