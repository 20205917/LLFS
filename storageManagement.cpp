//
// Created by 86136 on 2023/5/25.
//
#include "config.h"
#include "RunningSystem.h"

//һ�����̵�i�ڵ�
static struct dinode block_buf[BLOCKSIZ / DINODESIZ];

struct inode *ialloc(RunningSystem &runningSystem) {
    struct super_block &file_system = runningSystem.file_system;
    struct inode *temp_inode;

    unsigned int oneNum = BLOCKSIZ / DINODESIZ;

    if(file_system.s_pdinode == NICINOD){
        unsigned int count = 1;                     //�Ѿ�ɨ����
        unsigned int block_end_flag = 1;
        unsigned int cur_i = file_system.s_rdinode / oneNum;
        unsigned int cur_j = file_system.s_rdinode % oneNum;
        while (file_system.s_pdinode > 0 && count <= file_system.s_dinode_size){
            if (block_end_flag) {
                fseek(runningSystem.disk, DINODESTART + cur_i * DINODESIZ, SEEK_SET);
                fread(block_buf, 1, BLOCKSIZ,runningSystem.disk);
                block_end_flag = 0;
            }
            //�ҵ����п�
            while (block_buf[cur_j].di_mode == DIEMPTY && cur_j < oneNum) {
                cur_j++;
                count++;
            }
            //��һ�飬ɨ��һ��
            if (cur_j == oneNum) {
                block_end_flag = 1;
                cur_j = 0;
                cur_i++;
            }
            else {
                file_system.s_dinodes[--file_system.s_pdinode] = cur_i*oneNum+cur_j;
                count++;
            }
        }
        file_system.s_rdinode = cur_i*oneNum+cur_j;
    }

    //�����ڴ�i�ڵ�
    temp_inode = iget(file_system.s_dinodes[file_system.s_pdinode]);
    //д���ʼ������
    fseek(runningSystem.disk, DINODESTART + file_system.s_dinodes[file_system.s_pdinode] * DINODESIZ, SEEK_SET);
    fwrite(&temp_inode->dinode, 1, sizeof(struct dinode), runningSystem.disk);

    file_system.s_free_dinode_num--;
    file_system.s_pdinode++;
    file_system.s_fmod = SUPDATE;
    return temp_inode;
}


// ���ݶ�Ӧ��Ӳ��i�ڵ�id��ϵͳ�򿪱����ͷ�
// Ȼ���жϸ�i�ڵ��Ƿ�Ӧ�������i�ڵ�����
void ifree(int dinode_id, struct super_block &file_system) {
    {
        //??????������������ʱ��Ӧд���ͷŵ�dinode,��ֹ�´�����ʱ����û�и��ǡ�δ��ʱ����д�أ�
    }
    file_system.s_free_dinode_num++;
    if (file_system.s_pdinode > 0 ) {
        // δ�ﵽ�����п���
        file_system.s_dinodes[--file_system.s_pdinode] = dinode_id;
    }else {
        // ��������ж����ǽڵ��Ƿ���Ը���
        if (dinode_id < file_system.s_rdinode) {
            file_system.s_rdinode = dinode_id;
        }
    }
}

// �ͷŴ��̿�
// ���������Ŀռ����
// Ҫע���һ���鳤��ֻ��49����Ч������λΪ0
void bfree(int block_num, struct super_block &file_system, FILE* disk){
    if(file_system.s_pfree_block == NICFREE){
        // ջ���Ѿ�����
        // ��д�ش���
        fseek(disk, block_num * BLOCKSIZ, SEEK_SET);
        fwrite(file_system.s_free_blocks, 1, BLOCKSIZ, disk);
        // Ȼ����Ϊջ�����¿�ʼ
        file_system.s_pfree_block = 1;
        file_system.s_free_blocks[0] = block_num;
        file_system.s_free_block_size++;
    }
    else{
        // ջδ��
        file_system.s_free_blocks[file_system.s_pfree_block++] = block_num;
        file_system.s_free_block_size++;
    }
}

// ������̿�
unsigned int balloc(struct super_block &file_system, FILE *disk){
    if(file_system.s_free_block_size == 0){
        // �ռ䲻��
        return DISKFULL;
    }
    unsigned int block_num = file_system.s_free_blocks[file_system.s_pfree_block-1];
    if(file_system.s_free_block_size == 1){
        // ���ٷ�����ջ�գ���Ҫ�Ӵ��̼����鳤��
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