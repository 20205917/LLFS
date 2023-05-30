//
// Created by 86173 on 2023/5/25.
//
#include "RunningSystem.h"

//�鿴ĳ������i�ڵ�id��Ӧ���ڴ�i�ڵ��Ƿ����
inode* findHinode(int dinode_id){
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

hinode iget(unsigned int dinode_id){
    int inode_id = dinode_id % NHINO;
    hinode temp=findHinode(dinode_id);
    if(temp!=NULL)
        return temp;
    // �ڴ��в�����,��Ҫ����
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
    return true;
}



// �ӵ�ǰĿ¼�ҵ���һ���յ�λ��
// ���᷵���������е��±꣬��Ϊ-1����û�ҵ�
int seek_catalog_leisure(){//ԭ����iname
    // �Ӵ��̼���Ŀ¼
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
