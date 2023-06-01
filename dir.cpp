//
// Created by 86136 on 2023/6/1.
//
#include "RunningSystem.h"

// 创建文件夹，输入是文件路径
int mkdir(string &pathname) {
    string father_path;
    inode *catalog;
    string filename;
    if (pathname.find_last_of('/') == std::string::npos) {
        catalog = cur_dir_inode;
        filename = pathname;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);
        if (catalog == nullptr) {
            return -1;//无该路径，返回错误码
        }
    }
    if (!access(CHANGE, catalog))
        return -1;//权限不足，返回错误码
    //判断目录文件数据区是否有空闲
    //小于最大目录数，说明空闲
    dir *catalog_dir = (dir *) catalog->content;
    if (catalog_dir->size < DIRNUM) {
        //判断是否重复
        int leisure;
        for (int i = 0; i < DIRNUM; i++) {
            if (catalog_dir->files[i].d_name == filename) //如果有已经存在的文件夹，则返回错误码
                return -1;
            if (catalog_dir->files[i].d_index == 0)
                leisure = i;
        }


        //申请索引结点和硬盘数据区
        int new_d_index = ialloc(1);
        inode *new_inode = iget(new_d_index);
        new_inode->dinode.di_addr[0] = balloc();
        new_inode->dinode.di_mode = DIDIR;
        new_inode->dinode.di_size = sizeof(dir);
        new_inode->ifChange = 1;
        new_inode->dinode.di_number = 1;


        //初始化硬盘数据区(索引结点区在ialloc中初始化)
        new_inode->content = malloc(sizeof(dir));
        memset(new_inode->content, 0, sizeof(dir));
        strcpy(((dir *) new_inode->content)->files[0].d_name, "root");
        ((dir *) new_inode->content)->files[0].d_index = 1;
        strcpy(((dir *) new_inode->content)->files[1].d_name,
               father_path.substr(0, father_path.find_last_of('/') - 1).c_str());
        ((dir *) new_inode->content)->files[1].d_index = catalog->d_index;
        ((dir *) new_inode->content)->size = 2;
        //找到父目录空闲的目录项,写入文件名和文件磁盘结点
        strcpy(catalog_dir->files[leisure].d_name, filename.data());
        catalog_dir->files[leisure].d_index = new_d_index;
        catalog_dir->size++;
        catalog->ifChange = 1;
        return 1;
    }

    return -1;
}



int rmdir(string &pathname) {
    inode *father_catalog;
    string filename;
    if (pathname.find_last_of('/') == std::string::npos) {
        father_catalog = cur_dir_inode;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        father_catalog = find_file(father_path);
        if (father_catalog == nullptr)
            return -1;//无该路径，返回错误码
    }
    inode *catalog = find_file(pathname);
    if (catalog == nullptr)
        return -1;//无该路径，返回错误码
    if (!access(CHANGE, father_catalog))
        return -1;//权限不足，返回错误码
    auto *catalog_dir = (dir *) catalog->content;//得到路径的目录dir数据
    auto *father_dir = (dir *) father_catalog->content;//得到父目录的dir数据
    if (catalog_dir->size > 2) {
        return -1;//该路径的目录有内容，失败。
    } else {
        //将父目录里该项内容删除
        for (auto &i: father_dir->files) {
            if (i.d_index == catalog->d_index) {//查找该文件的下标
                i.d_index = 0;
                memset(i.d_name, 0, DIRSIZ);
                father_dir->size--;
                catalog->ifChange = 1;
                father_catalog->ifChange = 1;
                catalog->dinode.di_number--;
                break;
            }
        }
        return 1;
    }
}

//移动系统当前路径
int chdir(string &pathname) {
    int value=0;//判断是否是绝对路径
    if (judge_path(pathname) != 1) {
        return -1;
    }
    else {
        if (pathname[0] == '/'){
            if(pathname.size() == 1)
                pathname = "";
            pathname = "root" + pathname;
            value=1;
        }
        else if(pathname[0]=='r'&&pathname[1]=='o'&&pathname[2]=='o'&&pathname[3]=='t'&&(pathname.size() == 4 || pathname[4]=='/'))
            value = 1;
        inode *catalog = find_file(pathname);
        if (catalog == nullptr) {
            return -2;//无该路径，返回错误码
        }
        if (!access(CHANGE, catalog))
            return -3;//权限不足，返回错误码
        if(value){
            cur_path=pathname;
        } else{
            cur_path=cur_path+'/'+pathname;
        }
        cur_dir_inode = catalog;
    }
    return 1;
}

int show_dir() {
    if (!access(READ, cur_dir_inode))
        return -1;//权限不足，返回错误码

    for(int i = 2; i < DIRNUM; i++){
        unsigned int id = ((dir *) cur_dir_inode->content)->files[i].d_index;
        if(id != 0){
            bool inMemory = true;
            inode* tmp = findHinode(id);
            if(tmp == nullptr){
                inMemory = false;
                tmp = getDinodeFromDisk(id);
            }
            if(tmp->dinode.di_mode == DIDIR)
                std::cout << "<DIR>  ";
            else
                std::cout << "<FILE> ";
            std::cout << ((dir *) cur_dir_inode->content)->files[i].d_name << std::endl;
            if(!inMemory){
                free(tmp);
            }
        }
    }
    return 0;
}

int show_whole_dir(){
    // 从root开始
    std::cout << "<DIR>  " << "root" << std::endl;
    show_dir_tree(1, 1);
    return 0;
}

int show_dir_tree(unsigned int id, int depth){
    inode* tmp = findHinode(id);

    bool inMemory = true;
    if(tmp == nullptr){
        inMemory = false;
        tmp = getDinodeFromDisk(id);
    }
    dir* dirs = (dir*)(tmp->content);
    int size = dirs->size;
    for(int i = 2; i < DIRNUM; i++){
        if(size == 2)
            break;
        id = dirs->files[i].d_index;
        if(id != 0) {
            for(int j = 0; j < 4 * depth; j++){
                std::cout << " ";
            }
            bool _inMemory = true;
            inode* _tmp = findHinode(id);
            if(_tmp == nullptr){
                _inMemory = false;
                _tmp = getDinodeFromDisk(id);
            }
            if(_tmp->dinode.di_mode == DIDIR){
                std::cout << "<DIR>  ";
                std::cout << dirs->files[i].d_name << std::endl;
                show_dir_tree(dirs->files[i].d_index, depth + 1);
            }
            else{
                std::cout << "<FILE> ";
                std::cout << dirs->files[i].d_name << std::endl;
            }
            if(!_inMemory){
                free(_tmp->content);
                free(_tmp);
            }
            size--;
        }
    }
    if(!inMemory){
        free(tmp);
        free(dirs);
    }
    return 0;
}