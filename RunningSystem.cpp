//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include "RunningSystem.h"

RunningSystem::RunningSystem(){
    // ��Ӳ��
    disk = fopen("disk", "wb+");

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
        system_openfiles[i].fcb.d_ino = 0;
    }

    // ��ʼ��pwds
    fread(&pwds, sizeof(PWD), PWDNUM, disk);

    // ��ʼ��user_openfiles
    user_openfiles.clear();


    // ��ȡrootĿ¼
    // ����һ��i�ڵ�
    // ��ʼ��cur_path_inode
    cur_path_inode = iget(1, hinodes, disk);
    int size = cur_path_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for(i = 0; i < block_num; i++){
        id = cur_path_inode->dinode.di_addr[i];
        addr = DINODESTART + id * DINODESIZ;
        fseek(disk, addr, SEEK_SET);
        fread(&root+i*BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_path_inode->dinode.di_addr[block_num];
    addr = DINODESTART + id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread(&root+block_num*BLOCKSIZ, size-BLOCKSIZ*block_num, 1, disk);

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
            iput(system_openfiles[id_to_sysopen].fcb.d_ino,disk,file_system);
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
//�����ļ��У��������ļ�·�����ļ�����
bool RunningSystem::mkdir(const char *pathname, char *name)
{
    
    return false;
}

//��ʾ��ǰ�û�ss
string RunningSystem::whoami(){
    return cur_user;
}
RunningSystem::~RunningSystem(){
    fclose(disk);
}

