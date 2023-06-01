//
// Created by 86173 on 2023/5/25.
//
#include "RunningSystem.h"



//查看某个磁盘i节点id对应的内存i节点是否存在
inode* findHinode(unsigned int dinode_id){
    unsigned int inode_id = dinode_id % NHINO;
    if(hinodes[inode_id]->i_forw != nullptr) {
        hinode tmp = hinodes[inode_id]->i_forw;
        while (tmp) {
            if (tmp->d_index == dinode_id) {
                // 内存中已存在
                return tmp;
            } else
                tmp = tmp->i_forw;
        }
    }
    return nullptr;
}

//TODO iget iput,对于内容的释放

// 在内存i节点散列表根据硬盘i节点id查找
// 找不到就创建内存i节点
// 申请了额外空间,//插入内存i节点  情况: 1-Hash缓冲区对应桶号已达可容纳i节点数上限7  2-未达上限，直接插入
// 这里初始化了新的inode
hinode iget(unsigned int dinode_id){
    unsigned int inode_id = dinode_id % NHINO;
    hinode temp=findHinode(dinode_id);
    if(temp!=nullptr)
        return temp;
    // 内存中不存在,需要创建
    long addr = DINODESTART + dinode_id * DINODESIZ;
    auto newinode = (hinode)malloc(sizeof(struct inode));
    fseek(disk, addr, SEEK_SET);
    fread(&(newinode->dinode.di_number), DINODESIZ, 1, disk);
    temp=hinodes[inode_id];
    for(int i=0;temp->i_forw!=nullptr;i++){
        if(i==6)
            iput(hinodes[inode_id]);
        temp=temp->i_forw;
    }
    temp->i_forw=newinode;
    newinode->i_forw=nullptr;
    newinode->i_back=temp;
    newinode->d_index = dinode_id;
    newinode->ifChange = '0';
    newinode->content = nullptr;
    if(newinode->dinode.di_size == 0)
        return newinode;
    //
    newinode->content = malloc(newinode->dinode.di_size);
    memset(newinode->content,0,newinode->dinode.di_size);
    int i;
    for (i = 0; i < newinode->dinode.di_size / BLOCKSIZ ; i++) {
        fseek(disk, DATASTART + BLOCKSIZ * newinode->dinode.di_addr[i], SEEK_SET);
        fread(newinode->content, 1, BLOCKSIZ, disk);
    }
    fseek(disk, DATASTART + BLOCKSIZ * newinode->dinode.di_addr[i], SEEK_SET);
    fread(newinode->content, 1, newinode->dinode.di_size % BLOCKSIZ, disk);
    return newinode;
}

// 释放i节点回磁盘
// 如果被更改需要写回
// 如果硬连接数为0
// 好像这么写会有空间浪费
bool iput(inode* inode){
    if(inode->dinode.di_number == 0){
        // 删除对应的数据块
        if(inode->dinode.di_size>0){
            int blocks = inode->dinode.di_size / BLOCKSIZ + 1;
            for(int i= 0; i < blocks; i++){
                bfree(inode->dinode.di_addr[i]);
                inode->dinode.di_addr[i] = 0;
            }
        }
        inode->dinode.di_size = 0;
        //修改文件类型为空
        inode->ifChange = 1;
        inode->dinode.di_mode = DIEMPTY;

        // 释放磁盘i节点，只释放点
        ifree(inode->d_index);
    }

    // 写回磁盘
    if(inode->ifChange != 0){
        // 索引区操作
        long addr = DINODESTART + inode->d_index * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fwrite(&inode->dinode, DINODESIZ, 1, disk);

        // 数据区操作
        if(inode->dinode.di_size != 0){
            int blocks = inode->dinode.di_size / BLOCKSIZ + 1;
            for(int i= 0; i < blocks; i++){
                bfree(inode->dinode.di_addr[i]);
            }

            for(int i = 0; i < blocks; i++){
                unsigned int block_num = balloc();
            }
            write_data_back((void*)(inode->content), inode->dinode.di_addr, inode->dinode.di_size, disk);
        }

    }

    // 从散列表中释放
    if(inode->i_forw == nullptr)
        inode->i_back->i_forw = nullptr;
    else{
        inode->i_forw->i_back = inode->i_back;
        inode->i_back->i_forw = inode->i_forw;
    }
    free (inode->content);
    free(inode);
    return true;
}



