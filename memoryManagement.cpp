//
// Created by 86173 on 2023/5/25.
//
#include "RunningSystem.h"

//查看某个磁盘i节点id对应的内存i节点是否存在
inode* findHinode(int dinode_id){
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

hinode iget(unsigned int dinode_id){
    int inode_id = dinode_id % NHINO;
    hinode temp=findHinode(dinode_id);
    if(temp!=NULL)
        return temp;
    // 内存中不存在,需要创建
    long addr = DINODESTART + dinode_id * DINODESIZ;
    hinode newinode = (hinode)malloc(sizeof(struct inode));
    fseek(disk, addr, SEEK_SET);
    fread(&(newinode->dinode), DINODESIZ, 1, disk);
    temp=hinodes[inode_id];
    for(int i=0;temp->i_forw!=NULL;i++){
        if(i==6)
            iput(hinodes[inode_id]);
        temp=temp->i_forw;
    }
    temp->i_forw=newinode;
    newinode->i_forw=NULL;
    newinode->i_back=temp;
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
    return true;
}



// 从当前目录找到下一个空的位置
// 将会返回在数组中的下标，若为-1表明没找到
int seek_catalog_leisure(){//原来是iname
    // 从磁盘加载目录
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    struct dir* tmp = (struct dir*) malloc(sizeof(struct dir));
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
