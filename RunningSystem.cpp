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
        unsigned short id_to_sysopen = u->items[i].index_to_sysopen;
        if(--system_openfiles[id_to_sysopen].i_count == 0){
            inode* inode = iget(system_openfiles[id_to_sysopen].fcb.d_ino,hinodes,disk);
            iput(inode,disk,file_system);
        };
    }
    user_openfiles.erase(pwd);
    return;
}



//�жϻ����Ϸ��ԣ����ڣ����ȣ�λ�ڸ�Ŀ¼
//�������Ϸ�����
//������޸�.(�ݶ���
char *clean_path(const char *path){

inode* RunningSystem::createFile(string pathname, unsigned short di_mode){
    FCB  fcb;

    int i, j;
    //�����ļ�
    fcb = addFile(pathname);
    //�ļ��Ѿ�����
    if (fcb.d_ino != 0)
        return NULL;
    else
    {
        struct inode* inode = ialloc(*this);
        //��Ŀ¼�м���
        ;
        inode->dinode.di_mode = 1;
        inode->dinode.di_uid = user_openfiles.find(cur_user)->second->p_uid;
        inode->dinode.di_gid = user_openfiles.find(cur_user)->second->p_gid;
        inode->dinode.di_size = 0;
        inode->ifChange = 1;                //��Ҫ����д��
        return j;
    }
}

int RunningSystem::openFile(string pathname,unsigned short flags) {
    //�жϺϷ���
    string path = clean_path(pathname);
    //clean_path���
    if (path.empty())
        return -1;
    //Ѱ���ļ�
    FCB fcb = find_file(path);
    if(fcb.d_ino == 0){
        return -2;}

    //���̽ڵ�����ڴ�
    inode* inode = iget(fcb.d_ino,hinodes,disk);

    //��������ҷ���Ȩ��
    if(access(cur_user, inode)){
        return -3;
    }

    //����ϵͳ�򿪱�
    int index_to_system;
    for (index_to_system = 0; index_to_system < SYSOPENFILE; index_to_system++)
        if (system_openfiles[index_to_system].i_count == 0) break;
    if (index_to_system == SYSOPENFILE) {
        iput(inode,disk,file_system);
        return -4;
    }
    system_openfiles[index_to_system].i_count = 1;
    system_openfiles->fcb = fcb;

    //�����û��򿪱�
    user_open_table* u = user_openfiles.find(cur_user)->second;
    int fd;
    for (int fd = 0;fd < NOFILE;fd++){
        if(u->items[fd].f_count==0){
            u->items[fd].f_count = 1;
            u->items[fd].u_default_mode = flags;
            u->items[fd].f_offset = 0;
            u->items[fd].index_to_sysopen = index_to_system;
            u->items[fd].f_inode = inode;
            break;
        }
    };


    //����ļ�
    //TODO
//    /*if APPEND, free the block of the file before */
//    if (openmode & FAPPEND) {
//        for (index_to_system = 0; index_to_system < inode->di_size / BLOCKSIZ + 1; index_to_system++)
//            bfree(inode->di_addr[index_to_system]);
//        inode->di_size = 0;
//    }


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
// �����ļ��У��������ļ�·��
int RunningSystem::mkdir(string pathname)
{
    if(!is_dir(pathname)){
        return false;
    }
    else{
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0,pos-1);
        string file = pathname.substr(pos);
        inode* catalog =  find_file(father_path);
        if(catalog==NULL){
            return -1;//�޸�·�������ش�����
        }
        struct dir catalog_dir = get_dir(catalog->d_index);
        //�ж�Ŀ¼�ļ��������Ƿ��п���
        //С�����Ŀ¼����˵������
        if(catalog_dir.size<DIRNUM){
            //�ж��Ƿ��ظ�
            for(int i=0;i<DIRNUM;i++){
                if(catalog_dir.files[i].d_name==file){//������Ѿ����ڵ��ļ��У��򷵻ش�����
                    return -1;
                }
                else{
                    //������������Ӳ��������
                    int new_d_index = ialloc();
                    inode* new_inode = iget(new_d_index,hinodes,disk);
                    int block_amount = sizeof(dir)/BLOCKSIZ + 1;
                    for(int j=0;j<block_amount;j++){
                        new_inode->dinode.di_addr[j] = balloc(file_system,disk);
                    }
                    //��ʼ��Ӳ��������(�����������ialloc�г�ʼ��)
                    struct dir new_dir = get_dir(new_d_index);
                    string tmp = "root";
                    string_char(tmp,new_dir.files[0].d_name,tmp.length());
                    new_dir.files[0].d_index = 1;
                    new_dir.size = 0;
                    //�ҵ���Ŀ¼���е�Ŀ¼��,д���ļ������ļ����̽��
                    int leisure = seek_catalog_leisure(catalog,disk);
                    string_char(file,catalog_dir.files[leisure].d_name,file.length());
                    catalog_dir.files[leisure].d_index = new_d_index;
                    //����Ŀ¼���ڴ�i���д�����i��㣬�����ļ��е��ڴ�i���д�����i���
                    iput(catalog,disk,file_system);
                    iput(new_inode,disk,file_system);
                }
                return 1;
            }
        }
        else{//û�п��У�ʧ��
            return -1;
        }
    }
}
//�ƶ�ϵͳ��ǰ·��
int RunningSystem::chdir(string pathname)
{
    if(!is_dir(pathname)){
        return -1;
    }
    else{
        inode* catalog =  find_file(pathname);
        if(catalog==NULL){
            return -1;//�޸�·�������ش�����
        }
        cur_dir = get_dir(catalog->d_index);
        cur_dir_inode = catalog;
    }
    return 1;
}
int RunningSystem::show_dir(){
    for(int i=0;i<DIRNUM;i++){
        if(cur_dir.files[i].d_index!=0){
            cout<<cur_dir.files[i].d_name<<endl;//�����ǰ·���µ��ļ�����
        }
    }
}
int RunningSystem::rmdir(string pathname){
    if(!is_dir(pathname)){
        return -1;
    }
    else{

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

inode* RunningSystem::find_file(string addr){
    string Subaddr;
    int index;//��������ļ����ҵ���FCB��d_index
    int isInDir=0;//�ж��Ƿ���Ŀ¼��
    int first;//��һ�γ���'/'��λ��
    if(addr=="")
        return NULL;
    if(addr[addr.length()-1]=='/')
        return NULL;
    if(addr[0]=='/'){//����·��
        addr=addr.substr(1,addr.length());
        index=cur_dir.files[0].d_index;
        cur_dir=get_dir(index);
    }
    while(addr.find_first_of('/')!=string::npos){
        first=addr.find_first_of('/');
        Subaddr=addr.substr(0,first);
        addr=addr.substr(first+1,addr.length());
        for(int i=0;i<DIRNUM;i++){
            if(strcmp(cur_dir.files[i].d_name,Subaddr.c_str())==0){
                index=cur_dir.files[i].d_index;
                cur_dir=get_dir(index);
                isInDir=1;
                break;
            }
        }
        if(isInDir==0)
            return NULL;
        isInDir=0;
    }
    for(int i=0;i<DIRNUM;i++){
        if(strcmp(cur_dir.files[i].d_name,addr.c_str())==0)
            return iget(cur_dir.files[i].d_index, hinodes, disk);
    }
    return NULL;

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