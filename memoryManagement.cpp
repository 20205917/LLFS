//
// Created by 86173 on 2023/5/25.
//
#include "config.h"

//�鿴ĳ������i�ڵ�id��Ӧ���ڴ�i�ڵ��Ƿ����
hinode findHinode(int dinode_id , hinode* hinodes, FILE* disk){
    int inode_id = dinode_id % NHINO;
    if(hinodes[inode_id]->i_forw != NULL){
        hinode tmp = hinodes[inode_id]->i_forw;
        while(tmp){
            if(tmp->d_index == dinode_id){
                // �ڴ����Ѵ���
                return tmp;
            }
            else
                tmp = tmp->i_forw;
        }  
    return NULL; 
}


// ���ڴ�i�ڵ�ɢ�б����Ӳ��i�ڵ�id����
// �Ҳ����ʹ����ڴ�i�ڵ�
// �����˶���ռ�,//�����ڴ�i�ڵ�  ���: 1-Hash��������ӦͰ���Ѵ������i�ڵ�������7  2-δ�����ޣ�ֱ�Ӳ���
hinode iget(int dinode_id , hinode* hinodes, FILE* disk){
    int inode_id = dinode_id % NHINO;
    hinode tmp=findHinode(dinode_id,hinodes,disk);
    if(tmp!=NULL)
        return tmp;
    // �ڴ��в�����,��Ҫ����
    long addr = DINODESTART + dinode_id * DINODESIZ;
    hinode newinode = (hinode)malloc(sizeof(struct inode));
    fseek(disk, addr, SEEK_SET);
    fread(&(newinode->dinode.di_number), DINODESIZ, 1, disk);
    // �����ڴ�ɢ�б���Ϊhinodes[inode_id]����һ���ڵ�
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

    // �����ʼ��
    newinode->i_flag = 0;
    newinode->d_index = dinode_id;

    return newinode;
}

// �ͷ�i�ڵ�ش���
// �����������Ҫд��
// �����ɾ�����ڴ���Ĩ��(��Ҫ�ع�)
void iput(inode* inode, FILE* disk, struct super_block &file_system){
    if(inode->dinode.di_number != 0){
        // ��Ҫд��
        if(inode->i_flag != 0){
            long addr = DINODESTART + inode->d_index * DINODESIZ;
            fseek(disk, addr, SEEK_SET);
            fwrite(&inode->dinode, DINODESIZ, 1, disk);
        }
    }
    else{
        // ��ҪĨ����ռ�õĴ��̿�
        int num = inode->dinode.di_size / BLOCKSIZ;
        for(int i = 0; i < num; i++){
            // ����Ӧ���̿��ͷ�
            bfree(inode->dinode.di_addr[i], file_system, disk);
        }
        // ��ϵͳ�򿪱����ͷ�
        ifree(inode->d_index, file_system);
    }
    // ��ɢ�б����ͷ�
    if(inode->i_forw == nullptr)
        inode->i_back->i_forw = nullptr;
    else{
        inode->i_forw->i_back = inode->i_back;
        inode->i_back->i_forw = inode->i_forw;
    }
    free(inode);
}

// �ӵ�ǰĿ¼����name��Ӧ��i�ڵ�
// ���᷵���������е��±꣬��ΪDIRNUM����û�ҵ�
// ����������
unsigned int namei(char* name, hinode cur_path_inode, FILE* disk){
    // �Ӵ��̼���Ŀ¼�ļ�
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

    // ��ʼ����
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
// �ӵ�ǰĿ¼�ҵ���һ���յ�λ��
// ���᷵���������е��±꣬��Ϊ-1����û�ҵ�
int seek_catalog_leisure(inode* cur_path_inode, FILE* disk){//ԭ����iname
    // �Ӵ��̼���Ŀ¼
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

    // ��ʼ����
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
