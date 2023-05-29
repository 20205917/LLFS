//
// Created by 86173 on 2023/5/25.
//
#include "config.h"

//�鿴ĳ������i�ڵ�id��Ӧ���ڴ�i�ڵ��Ƿ����
inode* findHinode(int dinode_id , hinode* hinodes, FILE* disk){
    int inode_id = dinode_id % NHINO;
    if(hinodes[inode_id]->i_forw != NULL) {
        hinode tmp = hinodes[inode_id]->i_forw;
        while (tmp) {
            if (tmp->d_index == dinode_id) {
                // �ڴ����Ѵ���
                return tmp;
            } else
                tmp = tmp->i_forw;
        }
    }
    return NULL; 
}


// ���ڴ�i�ڵ�ɢ�б����Ӳ��i�ڵ�id����
// �Ҳ����ʹ����ڴ�i�ڵ�
// �����˶���ռ�,//�����ڴ�i�ڵ�  ���: 1-Hash��������ӦͰ���Ѵ������i�ڵ�������7  2-δ�����ޣ�ֱ�Ӳ���
inode* iget(int dinode_id , hinode* hinodes, FILE* disk){

    hinode tmp= findHinode(dinode_id, hinodes, disk);
    if(tmp!=NULL)
        return tmp;
    // �ڴ��в�����,��Ҫ����
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


    // �����ʼ��
    newinode->i_flag = 0;
    newinode->ifChange = 0;
    newinode->d_index = dinode_id;
    return newinode;
}

// �ͷ�i�ڵ�ش���
// �����������Ҫд��
// ���Ӳ������Ϊ0
bool iput(inode* inode){
    if(inode->dinode.di_number == 0){
        if(inode->dinode.di_size>0){
            //TODO �༶�����ͷ�������
        }
        //�޸��ļ�����Ϊ��
        inode->ifChange = 1;
        inode->dinode.di_mode = ENOTEMPTY;

        // �ͷŴ���i�ڵ㣬ֻ�ͷŵ�
        ifree(inode->d_index);
    }

    if(inode->ifChange != 0){
        long addr = DINODESTART + inode->d_index * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fwrite(&inode->dinode, DINODESIZ, 1, disk);
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
