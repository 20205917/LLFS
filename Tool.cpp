//
// Created by 86136 on 2023/5/25.
//
#include "config.h"

//路径是否合法
bool is_dir(string pathname){

    cout<<"您输入的地址不合法";
    return false;
}

//文件名是否合法
bool is_file(const char *filename){

}

//将数据区内容写回磁盘 内存中数据地址，硬盘索引数组，数据长度，文件指针
bool write_data_back(void *data_address, unsigned int *di_addr, int size, FILE *fp){
    int block_num = size / BLOCKSIZ;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        addr = DINODESTART + di_addr[i] * DINODESIZ;
        fseek(fp, addr, SEEK_SET);
        fwrite((char*)data_address+i*BLOCKSIZ, BLOCKSIZ, 1, fp);
    }
    addr = DINODESTART + di_addr[i] * DINODESIZ;
    fseek(fp, addr, SEEK_SET);
    fwrite((char*)data_address+i*BLOCKSIZ, size-block_num*BLOCKSIZ, 1, fp);
}
