//
// Created by 86173 on 2023/5/25.
//
#include "RunningSystem.h"



//�鿴ĳ������i�ڵ�id��Ӧ���ڴ�i�ڵ��Ƿ����
inode* findHinode(unsigned int dinode_id){
    unsigned int inode_id = dinode_id % NHINO;
    if(hinodes[inode_id]->i_forw != nullptr) {
        hinode tmp = hinodes[inode_id]->i_forw;
        while (tmp) {
            if (tmp->d_index == dinode_id) {
                // �ڴ����Ѵ���
                return tmp;
            } else
                tmp = tmp->i_forw;
        }
    }
    return nullptr;
}

//TODO iget iput,�������ݵ��ͷ�

// ���ڴ�i�ڵ�ɢ�б����Ӳ��i�ڵ�id����
// �Ҳ����ʹ����ڴ�i�ڵ�
// �����˶���ռ�,//�����ڴ�i�ڵ�  ���: 1-Hash��������ӦͰ���Ѵ������i�ڵ�������7  2-δ�����ޣ�ֱ�Ӳ���
// �����ʼ�����µ�inode
hinode iget(unsigned int dinode_id){
    unsigned int inode_id = dinode_id % NHINO;
    hinode temp=findHinode(dinode_id);
    if(temp!=nullptr)
        return temp;
    // �ڴ��в�����,��Ҫ����
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

// �ͷ�i�ڵ�ش���
// �����������Ҫд��
// ���Ӳ������Ϊ0
// ������ôд���пռ��˷�
bool iput(inode* inode){
    if(inode->dinode.di_number == 0){
        // ɾ����Ӧ�����ݿ�
        if(inode->dinode.di_size>0){
            int blocks = inode->dinode.di_size / BLOCKSIZ + 1;
            for(int i= 0; i < blocks; i++){
                bfree(inode->dinode.di_addr[i]);
                inode->dinode.di_addr[i] = 0;
            }
        }
        inode->dinode.di_size = 0;
        //�޸��ļ�����Ϊ��
        inode->ifChange = 1;
        inode->dinode.di_mode = DIEMPTY;

        // �ͷŴ���i�ڵ㣬ֻ�ͷŵ�
        ifree(inode->d_index);
    }

    // д�ش���
    if(inode->ifChange != 0){
        // ����������
        long addr = DINODESTART + inode->d_index * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fwrite(&inode->dinode, DINODESIZ, 1, disk);

        // ����������
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

    // ��ɢ�б����ͷ�
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



