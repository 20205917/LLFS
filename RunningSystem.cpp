//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include "RunningSystem.h"



RunningSystem::RunningSystem(){
    // ��Ӳ��
    disk = fopen("disk", "rb+");

    // ��ʼ��file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // ��ʼ��hinodes
    for(int i = 0; i < NHINO; i++){
        hinodes[i] = (hinode)malloc(sizeof(struct inode));
        hinodes[i]->i_forw = nullptr;
    }

    // ��ʼ��system_openfiles
    for(int i = 0; i < SYSOPENFILE; i++){
        system_openfiles[i].i_count = 0;
        system_openfiles[i].fcb.d_index = 0;
    }

    // ��ʼ��pwds
    fread(&pwds, sizeof(PWD), PWDNUM, disk);

    // ��ʼ��user_openfiles
    user_openfiles.clear();


    // ��ȡrootĿ¼��cur_dir��ǰĿ¼
    // ����һ��i�ڵ�
    // ��ʼ��cur_dir_inode
    cur_dir_inode = iget(1, hinodes, disk);
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DINODESTART + id * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char*)(&root)+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
        fread((char*)(&cur_dir)+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DINODESTART + id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char*)(&root)+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);
    fread((char*)(&cur_dir)+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);

};

int  RunningSystem::login(string pwd){
    int i,j;
    //����Ƿ���ƥ���PWD
    for(i = 0; i  < PWDNUM &&strcmp(pwd.c_str(),pwds[PWDNUM].password); i++);
    if(i==PWDNUM)
        return -1;

    //����Ƿ��Ѿ���¼
    if(user_openfiles.find(pwd) != user_openfiles.end()){
        return -2;
    }

    //�Ƿ��п�λ
    if(user_openfiles.size()<USERNUM){
        user_open_table* openTable = new user_open_table;
        openTable->p_uid = pwds[i].p_uid;
        openTable->p_gid = pwds[i].p_gid;
        memset(openTable->items,0,sizeof (user_open_item) * NOFILE);
        user_openfiles.insert(pair<string,user_open_table*>(pwd,openTable));
        //���õ�ǰ�ļ�Ϊ��Ŀ¼

        //cur_path_inode = iget(1);
    }
    return -3;
}
void RunningSystem::logout(string pwd){
    user_open_table* u = user_openfiles.find(pwd)->second;
    for (int i = 0; i < NOFILE; ++i) {
        //�ر�ÿ���ļ�
        unsigned short id_to_sysopen = u->items[i].id_to_sysopen;
        if(--system_openfiles[id_to_sysopen].i_count == 0){
            iput(system_openfiles[id_to_sysopen].fcb.d_index,disk,file_system);
        };
    }
    user_openfiles.erase(pwd);
    return;
}



//�жϻ����Ϸ��ԣ����ڣ����ȣ�λ�ڸ�Ŀ¼
//�������Ϸ�����
//������޸�.(�ݶ���
char *clean_path(const char *path){

    if (path == NULL || strlen(path) < 1 || strlen(path) > 1024 || path[0] != '/')
        return NULL;

    for (int i = 0; i < strlen(path); i++) {
        if (!((path[i] >= '0' && path[i] <= '9')
              ||(path[i] >= 'a' && path[i] <= 'z')
              || (path[i] >= 'A' && path[i] <= 'Z')
              ||path[i] == '.' || path[i] == '/'))    return NULL;
    }
    char *old_path = (char *) malloc(strlen(path) + 1);
    strcpy(old_path, path);
    char *new_path = (char *) malloc(strlen(path) + 1);
    memset(new_path, 0, strlen(path) + 1);
    int i = 0;

    int k = 0;
//    �����ļ���Ŀ¼������ <= 32 �ֽ�
    char *temp = strtok(old_path, "/");
    while (temp != NULL) {
        if (strlen(temp) > 32) {
            k = 1;
            break;
        }
        new_path[i++] = '/';
        for(int j = 0;j < strlen(temp); j++)
            new_path[i++] = temp[j];
        temp = strtok(NULL, "/");
    }
    free(old_path);
    if(k == 1){
        //free(new_path);
        new_path =NULL;
        return NULL;
    }
    return new_path;
}

int RunningSystem::openFile(const char *pathname, int flags) {
    //�жϺϷ���
    char *path = NULL;
    //clean_path���
    if ((path = clean_path(pathname)) == NULL)
        return -1;
    //Ѱ���ļ�
    inode* in = find_file(path);

    if(in == nullptr){
        free(path);
        return -2;
    }
    //��������ҷ���Ȩ��
    if(access(cur_user, in)){
        free(path);
        return -3;
    }
    iget(1,hinodes,disk);

    struct user_open_item{
        unsigned int f_count;               //ʹ�ý�����
        unsigned short u_default_mode;      //�򿪷�ʽ
        struct inode *f_inode;              //�ڴ�i�ڵ�ָ��
        unsigned long f_offset;             //�ļ�ƫ�������ļ�ָ�룩
        unsigned short id_to_sysopen;       //ϵͳ�򿪱�����
    };

    user_open_table* u = user_openfiles.find(cur_user)->second;
    int fd = 0;
    while (u->items[++fd]. != NULL) ;

    return fd;
}
struct dir RunningSystem::get_dir(int d_index)
{
    inode *dir_inode = iget(d_index,hinodes,disk);
    // �Ӵ��̼���Ŀ¼
    struct dir work_dir;
    work_dir.size=dir_inode->dinode.di_size/(DIRSIZ+2);
    int i=0;
    for(i= 0; i<work_dir.size/(BLOCKSIZ/(DIRSIZ+2));i++)
    {
        fseek(disk,DATASTART+BLOCKSIZ * dir_inode->dinode.di_addr[i],SEEK_SET);
        fread(&work_dir.files[(BLOCKSIZ/(DIRSIZ+2)) * i],1, BLOCKSIZ,disk);
    }
    fseek(disk, DATASTART+BLOCKSIZ * dir_inode->dinode.di_addr[i],SEEK_SET);
    fread(&work_dir.files[(BLOCKSIZ)/(DIRSIZ+2) * i], 1,dir_inode->dinode.di_size % BLOCKSIZ, disk);
    return work_dir;
}
// �����ļ��У��������ļ�·�����ļ�����
int RunningSystem::mkdir(string pathname, char *name)
{
    if(!is_dir(pathname)){
        return false;
    }
    else{
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0,pos-1);
        string file = pathname.substr(pos);
        inode* catalog =  find_file(pathname);
        //�ж�Ŀ¼�ļ��������Ƿ��п���
        if(seek_catalog_leisure(catalog,disk)!=-1){
            //�ж��Ƿ��ظ�
            
        }
        else{//û�п��У�ʧ��
            return -1;
        }
    }
}


//��ʾ��ǰ�û�ss
string RunningSystem::whoami(){
    return cur_user;
}

RunningSystem::~RunningSystem(){
    fclose(disk);
}
// Ŀ¼�ļ��������Ƿ��п��У��Ƿ��ڸ�Ŀ¼�����������Ѵ����ļ�(findfile()ʵ��)���п���
// �������i��㣬��ʼ������i��㣬����i���д�ش��̣���Ŀ¼������д�ش��̡�
// �������i�ڵ�δʵ��
inode* RunningSystem::createFile(const char *pathname, unsigned short di_mode)
{
    // �ж��ļ����Ƿ�Ϸ�
    if(!is_file(pathname)){
        return nullptr;
    }
    hinode res_inode = (hinode)malloc(sizeof(struct inode));
    res_inode->d_index = 0;
    // �Ƿ��ڸ�Ŀ¼�����������Ѵ����ļ�
    if(find_file(const_cast<char *>(pathname))->d_index != 0){
        return res_inode;
    }
    // �Ƿ��п���
    int index = iname(const_cast<char *>(pathname), cur_dir_inode, disk);
    if(index == DIRNUM)
        return nullptr;
    // �������i���
    struct inode* new_inode = ialloc();
    // ��ʼ������i�ڵ�
    new_inode->dinode.di_mode = di_mode;
    new_inode->dinode.di_size = 0;
    // ����i�ڵ�д�ش���
    long addr = DINODESTART + new_inode->d_index * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(new_inode->dinode), DINODESIZ, 1, disk);
    // ����Ŀ¼��
    cur_dir.files[index].d_index = new_inode->d_index;
    cur_dir.size++;
    // ��Ŀ¼д�ش���������
    write_data_back((void*)(&cur_dir), cur_dir_inode->dinode.di_addr, sizeof(struct dir), disk);
    return new_inode;
}

// �ļ����Ƿ�Ϸ�����Ҫfindfile���ж��Ƿ���Ȩ��
// �޸ĸ�Ŀ¼��������д����̣�iput()ɾ���ļ�
// falseɾ��ʧ�� trueɾ���ɹ�
// Ȩ��δʵ�� iputδʵ��
bool RunningSystem::deleteFile(const char *pathname){
    // �ж��ļ����Ƿ�Ϸ�
    if(!is_file(pathname)){
        return false;
    }
    hinode res_inode = find_file(const_cast<char *>(pathname));
    // �Ƿ��ڸ�Ŀ¼���������и��ļ�
    if(res_inode->d_index == 0){
        return false;
    }
    // �ж��û��Ը�Ŀ¼��дȨ�޺�ִ��Ȩ���Ƿ�
    // if(access())

    // �޸ĸ�Ŀ¼������
    // ����Ŀ¼��
    unsigned int index = namei(const_cast<char *>(pathname), cur_dir_inode, disk);
    cur_dir.files[index].d_index = 0;
    cur_dir.size--;
    // д�����
    write_data_back((void*)(&cur_dir), cur_dir_inode->dinode.di_addr, sizeof(struct dir), disk);
    // iput()ɾ���ļ�
    // iput(res_inode);

    return true;
}

// �ر�һ���Ѿ����û����˵��ļ�

void RunningSystem::closeFile(const char *pathname){
    // �ж��ļ����Ƿ�Ϸ�
    if(!is_file(pathname)){
        return;
    }
    // ��ȡ�û��Ĵ򿪱�
    user_open_table* userOpenTable = user_openfiles[cur_user];
    // ��ȡ�û�uid
    unsigned short p_uid = userOpenTable->p_uid;

    // ������ѯ���ļ�
    unsigned short id;
    bool found = false;
    int i;
    for(i = 0; i < NOFILE; i++){
        if(userOpenTable->items[i].f_inode == nullptr){
            continue;
        }
        id = userOpenTable->items[i].id_to_sysopen;
        if(!strcmp(system_openfiles[id].fcb.d_name, pathname) && system_openfiles[id].i_count != 0){
            found = true;
            break;
        }
    }
    // �û�û�д򿪸��ļ�
    if(!found)
        return;

    // ȷ������
    // �ͷŸ��ļ����ڴ�i�ڵ�
    iput(userOpenTable->items[i].f_inode, disk, file_system);
    userOpenTable->items[i].f_inode = nullptr;
    // ϵͳ�򿪱��ж��Ƿ���Ҫ�ر�
    system_openfiles[id].i_count--;
    if(system_openfiles[id].i_count == 0){
        // ������ļ���ϵͳ�򿪱��йر�
        system_openfiles[id].fcb.d_index = 0;
    }
}

// ���û��Դ򿪵��ļ��ж�ȡ����
// ���ַ���ʽ��������
// û��ʵ��Ȩ���ж�
std::string RunningSystem::readFile(const char *pathname){
    // �ж��ļ����Ƿ�Ϸ�
    if(!is_file(pathname)){
        return {};
    }

    // �ж��û��Ը��ļ��Ƿ��ж�Ȩ��
    // if(access())

    // �ж��ļ��Ƿ��û���
    // ��ȡ�û��Ĵ򿪱�
    user_open_table* userOpenTable = user_openfiles[cur_user];
    // ��ȡ�û�uid
    unsigned short p_uid = userOpenTable->p_uid;

    // ������ѯ���ļ�
    unsigned short id;
    bool found = false;
    int i;
    for(i = 0; i < NOFILE; i++){
        if(userOpenTable->items[i].f_inode == nullptr){
            continue;
        }
        id = userOpenTable->items[i].id_to_sysopen;
        if(!strcmp(system_openfiles[id].fcb.d_name, pathname) && system_openfiles[id].i_count != 0){
            found = true;
            break;
        }
    }
    // �û�û�д򿪸��ļ�
    if(!found)
        return {};

    // ȷ������
    unsigned int dinode_id = system_openfiles[id].fcb.d_index;
    // �����������i�ڵ�
    struct dinode* pdinode = (struct dinode*) malloc(sizeof(struct dinode));
    long addr = DINODESTART + dinode_id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread(pdinode, DINODESIZ, 1, disk);
    // ׼����ȡ����
    unsigned short size = pdinode->di_size;
    int block_num = size / BLOCKSIZ;
    char* res = (char*)malloc(size);
    for(i = 0; i < block_num; i++){
        addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread(res+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread(res+i*BLOCKSIZ, size-block_num*BLOCKSIZ, 1, disk);
    return res;
}

// Ȩ��δʵ��
bool RunningSystem::writeFile(const char *pathname, int write_mode, std::string content){
    // �ж��ļ����Ƿ�Ϸ�
    if(!is_file(pathname)){
        return {};
    }
    // �ж��û��Ը��ļ��Ƿ���дȨ��
    // if(access())

    // �ж��ļ��Ƿ��û���
    // ��ȡ�û��Ĵ򿪱�
    user_open_table* userOpenTable = user_openfiles[cur_user];
    // ��ȡ�û�uid
    unsigned short p_uid = userOpenTable->p_uid;

    // ������ѯ���ļ�
    unsigned short id;
    bool found = false;
    int i;
    for(i = 0; i < NOFILE; i++){
        if(userOpenTable->items[i].f_inode == nullptr){
            continue;
        }
        id = userOpenTable->items[i].id_to_sysopen;
        if(!strcmp(system_openfiles[id].fcb.d_name, pathname) && system_openfiles[id].i_count != 0){
            found = true;
            break;
        }
    }
    // �û�û�д򿪸��ļ�
    if(!found)
        return false;

    // ����ģʽ
}