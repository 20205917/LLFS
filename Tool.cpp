//
// Created by 86136 on 2023/5/25.
//
#include <algorithm>
#include "RunningSystem.h"

//路径是否合法
//目录是否合法  0- 错误输入 1-目录 2-非目录文件
int judge_path(string pathname) {
    string Subpathname;
    int first;//第一次出现'/'的位置
    if (pathname == "" || (pathname[pathname.length() - 1] == '/'&&pathname.length()!=1))        //最后一个字符为'/'以及string为空时错
        return 0;
    if (pathname[0] == '/')//绝对路径
        pathname = pathname.substr(1, pathname.length());      //除去第一个字符'/'
    //依次顺地址往下一个目录，判断文件名是否合法以及是否出现连续的'/'
    while (pathname.find_first_of('/') != string::npos) {
        if (pathname[0] == '/')        //说明出现了连续的'/'
            return 0;
        first = pathname.find_first_of('/');
        Subpathname = pathname.substr(0, first);
        pathname = pathname.substr(first + 1, pathname.length());
        if (Subpathname.find('.') != string::npos)     //目录名中出现'.'
            return 0;
    }
    //走出上面循环后的pathname为最后一级的文件名
    if (pathname.find('.') == string::npos)        //说明不含'.'，为目录名
        return 1;

    int count = std::count(pathname.begin(), pathname.end(), '.');
    if (count > 1)         //文件名中不可以含有一个以上的'.'
        return 0;
    return 2;
}

//将数据区内容写回磁盘 内存中数据地址，硬盘索引数组，数据长度，文件指针
void write_data_back(void *data_address, unsigned short *di_addr, int size, FILE *fp){
    int block_num = size / BLOCKSIZ;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        addr = DATASTART + di_addr[i] * BLOCKSIZ;
        fseek(fp, addr, SEEK_SET);
        fwrite((char*)data_address+i*BLOCKSIZ, BLOCKSIZ, 1, fp);
    }
    addr = DATASTART + di_addr[i] * BLOCKSIZ;
    fseek(fp, addr, SEEK_SET);
    fwrite((char*)data_address+i*BLOCKSIZ, size-block_num*BLOCKSIZ, 1, fp);
}

//将数据区内容写回磁盘 内存中数据地址，硬盘索引数组，数据长度，文件指针
void read_data_from(void *data_address, unsigned int *di_addr, int size, FILE *fp){
    int block_num = size / BLOCKSIZ;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        addr = DATASTART + di_addr[i] * BLOCKSIZ;
        fseek(fp, addr, SEEK_SET);
        fread((char*)data_address+i*BLOCKSIZ, BLOCKSIZ, 1, fp);
    }
    addr = DATASTART + di_addr[i] * BLOCKSIZ;
    fseek(fp, addr, SEEK_SET);
    fread((char*)data_address+i*BLOCKSIZ, size-block_num*BLOCKSIZ, 1, fp);
}