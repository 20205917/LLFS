//
// Created by 86136 on 2023/6/1.
//

#include "RunningSystem.h"

//打开文件
int open_file(string &pathname, int operation) {
    if (judge_path(pathname) != 2)
        return -1;                                               //不是文件格式，返回错误码
    inode *catalog;
    string filename;
    if (pathname.find_last_of('/') == string::npos) {//当前目录的子文件     绝对路径
        catalog = cur_dir_inode;
        filename = pathname;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);//获取目录文件的内存索引节点
        if (catalog == nullptr) {
            return -1;//无该路径，返回错误码
        }
    }
    if (!access(READ, catalog))
        return -1;                                                  //权限不足，返回错误码
    auto *catalog_dir = (dir *) catalog->content;
    unsigned int file_index;//文件的硬盘i结点id
    int leisure = -1;//目录下的空闲索引
    int i;
    for (i = 0; i < DIRNUM; i++) {
        if (catalog_dir->files[i].d_name == filename) {//查找成功
            //查找成功，获取磁盘索引号
            file_index = catalog_dir->files[i].d_index;
            break;
        }
        if (catalog_dir->files[i].d_index == 0)
            leisure = i;
    }
    inode * new_inode;
    if (i == DIRNUM) {//没查找成功
        if (operation != BUILD_OPEN)//如果不是创建打开，就返回错误码，未找到文件
            return -2;
        else {//是创建打开
            if (leisure == -1)                                             //若目录已满，则返回错误码
                return -3;
            //创建新结点
            file_index = ialloc(1);
            new_inode = iget(file_index);
            new_inode->dinode.di_mode = DIFILE;
            new_inode->ifChange = 1;
            //修改目录的数据
            strcpy(catalog_dir->files[leisure].d_name, filename.data());
            catalog_dir->files[leisure].d_index = file_index;
            catalog_dir->size++;
            catalog->ifChange = 1;
            //写回文件磁盘i结点内容，写回目录磁盘i结点内容
        }
    }
    else// 查找成功，找到内存索引节点
        new_inode = iget(file_index);
    //修改系统打开文件表
    short sys_leisure = 0;
    for (; sys_leisure < SYSOPENFILE; sys_leisure++) {//找到空闲
        if (system_openfiles[sys_leisure].i_count == 0) {
            system_openfiles[sys_leisure].i_count++;
            system_openfiles[sys_leisure].fcb.d_index = file_index;
            strcpy(system_openfiles[sys_leisure].fcb.d_name, filename.data());
            break;
        }
    }
    if (sys_leisure == SYSOPENFILE)
        return -4;//没找到系统打开表空闲的表项
    //修改用户文件打开表
    user_open_table *T = user_openfiles.find(cur_user)->second;
    int usr_leisure = 0;
    for (; usr_leisure < SYSOPENFILE; usr_leisure++) {
        if (T->items[usr_leisure].f_count == 0) {
            T->items[usr_leisure].f_count++;
            if (operation == FP_TAIL_OPEN)
                T->items[usr_leisure].f_offset = iget(file_index)->dinode.di_size;
            else
                T->items[usr_leisure].f_offset = 0;
            T->items[usr_leisure].index_to_sysopen = sys_leisure;
            T->items[usr_leisure].u_default_mode = operation;
            T->items[usr_leisure].f_inode = new_inode;
            return usr_leisure;//返回用户打开表索引
        }
    }
    return -5;//没找到用户打开表空闲表项
}



inode *find_file(string addr) {
    dir *temp_dir = (dir *) cur_dir_inode->content;   //用于目录变更
    unsigned int index; //保存根据文件名找到的FCB的d_index
    int isInDir = 0;    //判断是否在目录中
    hinode final;//可以保存退出循环后最后一级的文件或目录的内存i结点
    if (addr[0] == '/') {//绝对路径
        addr = addr.substr(1, addr.length());
        temp_dir = (dir *) (iget(1)->content);
    }
    char *ADDR = new char[addr.length() + 1];
    addr.copy(ADDR, addr.length(), 0); //addr内容复制到ADDR[]
    *(ADDR + addr.length()) = '\0'; //末尾补上'/0'
    char *token = std::strtok(ADDR, "/");
    while (true) {
        for (auto &file: temp_dir->files) {
            if (strcmp(file.d_name, token) == 0) {
                index = file.d_index;
                isInDir = 1;
                final = iget(file.d_index);
                break;
            }
        }
        if (isInDir == 0) {//找不到符合的文件名
            free(ADDR);
            return nullptr;
        }
        isInDir = 0;

        token = std::strtok(nullptr, "/");
        if (token == nullptr)
            break;
        temp_dir = (dir *) iget(index)->content;
    }
    free(ADDR);
    return final;
}



//硬连接
int hard_link(string &pathname,string &newname){
    string father_path;
    inode *filea;
    string filename;
    if (pathname[0] == '/')
        pathname = "root" + pathname;
    filea = find_file(pathname);
    if (filea == nullptr)
        return -1;//无该路径，返回错误码
    if (!access(READ, filea))
        return -1;//权限不足，返回错误码
    inode * catalog_b = cur_dir_inode;
    if(((dir*)catalog_b->content)->size==DIRNUM)
        return -1;//目录无空闲
    for(int leisure=0;leisure<DIRNUM;leisure++){
        if(((dir*)catalog_b->content)->files[leisure].d_index==0){
            strcpy(((dir*)catalog_b->content)->files[leisure].d_name,newname.data());
            ((dir*)catalog_b->content)->files[leisure].d_index=filea->d_index;
            ((dir*)catalog_b->content)->size++;
            filea->dinode.di_number++;
            filea->ifChange=1;
            break;
        }
    }
    return 1;
}

// 硬链接次数初始化为1
// 需要考虑文件偏移量，此处未实现
int createFile(string pathname){
//    if (judge_path(pathname) != 2)
//        return -1;                                               //不是文件格式，返回错误码
    inode *catalog;
    string filename;
    if (pathname.find_last_of('/') == string::npos) {//当前目录的子文件     绝对路径
        catalog = cur_dir_inode;
        filename = pathname;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);//获取目录文件的内存索引节点
        if (catalog == nullptr)
            return -1;//无该路径，返回错误码
    }
    if (!access(Write, catalog))
        return -1;                                                  //权限不足，返回错误码
    auto *catalog_dir = (dir *) catalog->content;
    unsigned int file_index;//文件的硬盘i结点id
    int leisure = -1;//目录下的空闲索引
    int i;
    for (i = 0; i < DIRNUM; i++) {
        if (catalog_dir->files[i].d_name == filename) {//查找成功
            //直接返回
            return -2;
        }
        if (catalog_dir->files[i].d_index == 0)
            leisure = i;
    }
    inode * new_inode;
    if (i == DIRNUM) {//没查找成功
        if (leisure == -1)                                             //若目录已满，则返回错误码
            return -3;
        //创建新结点
        file_index = ialloc(1);
        new_inode = iget(file_index);
        new_inode->dinode.di_mode = DIFILE;
        new_inode->ifChange = 1;
        //
        new_inode->dinode.di_number = 1;
        //修改目录的数据
        strcpy(catalog_dir->files[leisure].d_name, filename.data());
        catalog_dir->files[leisure].d_index = file_index;
        catalog_dir->size++;
        catalog->ifChange = 1;

    }
    return 0;//创建成功
}

/* 文件名是否合法，需要findfile，判断是否有权限
// 修改父目录数据区并写入磁盘，iput()删除文件
// false删除失败 true删除成功
// 权限未实现 iput未实现*/
int deleteFile(string pathname) {
    inode *catalog;
    string filename;
    if (pathname.find_last_of('/') == string::npos) {//当前目录的子文件     绝对路径
        catalog = cur_dir_inode;
        filename = pathname;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);//获取目录文件的内存索引节点
        if (catalog == nullptr)
            return -2;//无该路径，返回错误码
    }
    if (!access(READ, catalog))
        return PERMISSION_DD;                                                  //权限不足，返回错误码
    auto *catalog_dir = (dir *) catalog->content;
    unsigned int file_index;//文件的硬盘i结点id
    int i;
    for (i = 0; i < DIRNUM; i++) {
        if (catalog_dir->files[i].d_name == filename) {//查找成功
            //查找成功，获取磁盘索引号
            file_index = catalog_dir->files[i].d_index;
            break;
        }
    }
    inode *file_inode;
    if (i == DIRNUM) {//没查找成功
        return NOT_FOUND;//无该文件，删除失败，返回错误码
    }
    //查找成功，找到内存索引节点
    file_inode = iget(file_index);
    //修改系统打开文件表
    for (auto & system_openfile : system_openfiles) {//找系统打开表的表项
        if (system_openfile.fcb.d_index == file_index && system_openfile.i_count != 0)
            return -1;//该文件正在被系统打开
    }
    //删除文件，若文件硬连接次数为0，则释放将索引节点中内容指针置为空
    file_inode->dinode.di_number--;
    if (file_inode->dinode.di_number == 0) {
        if(file_inode->content){
            free(file_inode->content);
            file_inode->content = nullptr;
        }
        file_inode->content = nullptr;
        catalog_dir->files[i].d_index = 0;
        catalog_dir->size--;
        catalog->ifChange = 1;
    }
    file_inode->ifChange = 1;
    return 0;//成功删除
}

// 关闭一个已经被用户打开了的文件

void closeFile(const string &pathname) {
    // 判断文件名是否合法
    if (judge_path(pathname) != 2) {
        return;
    }
    // 获取用户的打开表
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // 获取用户uid


    // 遍历查询该文件
    unsigned short id;
    bool found = false;
    int i;
    for (i = 0; i < NOFILE; i++) {
        if (userOpenTable->items[i].f_inode == nullptr) {
            continue;
        }
        id = userOpenTable->items[i].index_to_sysopen;
        if (!strcmp(system_openfiles[id].fcb.d_name, pathname.c_str()) && system_openfiles[id].i_count != 0) {
            found = true;
            break;
        }
    }
    // 用户没有打开该文件
    if (!found)
        return;

    // 确定打开了
    // 释放该文件的内存i节点
    iput(userOpenTable->items[i].f_inode);
    userOpenTable->items[i].f_inode = nullptr;
    // 系统打开表判断是否需要关闭
    system_openfiles[id].i_count--;
    if (system_openfiles[id].i_count == 0) {
        // 将这个文件从系统打开表中关闭
        system_openfiles[id].fcb.d_index = 0;
    }
}

int file_seek(int fd,int offset,int fseek_mode){
    user_open_table *T = user_openfiles.find(cur_user)->second;
    int cur_offset = T->items[fd].f_offset;
    int file_capacity = T->items[fd].f_inode->dinode.di_size;
    switch (fseek_mode)
    {
        case HEAD_FSEEK://从头移动
            cur_offset = offset;
            break;
        case CUR_SEEK://从当前移动
            cur_offset += offset;
            break;
        case LAST_SEEK://从末尾移动
            cur_offset = file_capacity + offset;
        default:
            cur_offset += offset;
            break;
    }
    if(cur_offset<0)
        return -1;//移动偏移量出界
    if(cur_offset>file_capacity){
        file_capacity = cur_offset;
        T->items[fd].f_inode->dinode.di_size = file_capacity;
        void *new_content = malloc(file_capacity);
        memset(new_content,0,file_capacity);
        strcpy((char *)new_content,(char *)T->items[fd].f_inode->content);
        free(T->items[fd].f_inode->content);
        T->items[fd].f_inode->content = new_content;
    }
    T->items[fd].f_offset = cur_offset;
    return cur_offset;
}


int writeFile(int fd, const string& content) {
    //判断文件是否被用户打开

    // 获取用户的打开表
    user_open_table *userOpenTable = user_openfiles[cur_user];

    // 使用fd获取打开文件
    struct user_open_item opened_file = userOpenTable->items[fd];
    // 为0说明读取错误
    if(opened_file.f_count == 0)
        return -1; // 文件标识符错误

    hinode file_inode = opened_file.f_inode;
    file_inode->ifChange = 1;
    unsigned long offset = opened_file.f_offset;


    // 写文件
    std::string tmp = string ((char*)file_inode->content,file_inode->dinode.di_size);
    tmp = tmp.substr(0, offset);
    tmp += content;

    free(file_inode->content);
    file_inode->content = (char*) malloc(tmp.size() + 1);
    strcpy((char*)file_inode->content, tmp.c_str());

    file_inode->dinode.di_size = tmp.size() + 1;
    userOpenTable->items[fd].f_offset = tmp.size();
    return 1;
}

// 从用户以打开的文件中读取内容
// 以字符形式返回内容
// 没有实现权限判断
string readFile(int fd,int len) {
    //判断文件是否被用户打开
    // 获取用户的打开表
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // 使用fd获取打开文件
    struct user_open_item opened_file = userOpenTable->items[fd];

    // 为0说明读取错误
    if(opened_file.f_count == 0)
        return "";

    hinode file_inode = opened_file.f_inode;

    // 判断用户对该文件是否有读权限
    if(!access(Read, file_inode))
        return "";

    if(opened_file.f_offset+len > file_inode->dinode.di_size){
        len = file_inode->dinode.di_size - opened_file.f_offset;
    }

    //读取
    string result = string((char*)file_inode->content+opened_file.f_offset,len);

    //修改offset
    userOpenTable->items[fd].f_offset +=len;

    return result;
}


int change_file_owner(string& pathname, int uid){
    inode* tmp =  find_file(pathname);
    if(tmp == nullptr)
        return -1; // 路径无效
    int o_uid = tmp->dinode.di_uid;

    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, cur_user.c_str())){
            if(pwds[i].p_uid == 0){
                pwds[i].p_uid = uid;
                return 0; // 管理员可以直接改
            }
            else if(o_uid != pwds[i].p_uid){
                return -2; // 其他非创建者无法修改
            }
            // 创建者也可以改
            pwds[i].p_uid = uid;
            return 0;
        }
    }
    return -1;
}

int change_file_group(string& pathname, int gid){
    inode* tmp =  find_file(pathname);
    if(tmp == nullptr)
        return -1; // 路径无效
    int o_gid = tmp->dinode.di_gid;

    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, cur_user.c_str())){
            if(pwds[i].p_uid == 0){
                pwds[i].p_gid = gid;
                return 0; // 管理员可以直接改
            }
            else if(o_gid != pwds[i].p_gid){
                return -2; // 其他非同组用户无法修改
            }
            // 同组用户也可以改
            pwds[i].p_gid = gid;
            return 0;
        }
    }
    return -1;
}
