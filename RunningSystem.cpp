//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include "RunningSystem.h"

void initial(){
    // ��Ӳ��
    disk = fopen("disk", "rb+");
    char nothing[BLOCKSIZ] = {0};
    fseek(disk, 0, SEEK_SET);
    fwrite(nothing, BLOCKSIZ, FILEBLK + DINODEBLK + 2, disk);

    // ��ʼ��������
    // ��ʱֻ��root��Ӧ�Ĵ���i�ڵ�����ݿ�
    // ռһ������i�ڵ��һ�����ݿ飬��Ŷ�Ϊ1
    file_system.s_block_size = FILEBLK - 1;
    file_system.s_dinode_size = BLOCKSIZ / DINODESIZ * DINODEBLK;
    file_system.s_free_dinode_num = file_system.s_dinode_size - 1;
    for(int i = 0; i < NICINOD; i++){
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = 0;
    for(int i = FILEBLK; i > 1; i--){
        bfree(i);
    }
    file_system.s_fmod = '0';

    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);

    // ��ʼ��root����i�ڵ�
    cur_dir_inode = (hinode) malloc(sizeof(struct inode));
    cur_dir_inode->dinode.di_number = 0;
    cur_dir_inode->dinode.di_mode = DIDIR;
    cur_dir_inode->dinode.di_uid = 0;
    cur_dir_inode->dinode.di_gid = 0;
    cur_dir_inode->dinode.di_size = sizeof(struct dir);
    cur_dir_inode->dinode.di_addr[0] = 0;

    long addr = DINODESTART + 1 * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(cur_dir_inode->dinode), DINODESIZ, 1, disk);

    // ��ʼ��rootĿ¼���ݿ�����
    root.size = 2;
    // ��Ŀ¼
    root.files[0].d_index = 1;
    strcpy(root.files[0].d_name, "root");
    // ��Ŀ¼
    root.files[1].d_index = 1;
    strcpy(root.files[1].d_name, "root");
    for(int i = 2; i < DIRNUM; i++){
        root.files[i].d_index = 0;
    }

    addr = DATASTART;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(root), sizeof(struct dir), 1, disk);

    // admin�˻�
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // һ����ͨ�˻�
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // ���
    for(int i = 2; i < PWDNUM; i++){
        strcpy(pwds[i].password, "");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
    fclose(disk);
}

void install() {
    // ��Ӳ��
    disk = fopen("disk", "rb+");

    // ��ʼ��file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // ��ʼ��hinodes
    for (auto &i: hinodes) {
        i = (hinode) malloc(sizeof(struct inode));
        memset(i,0,sizeof (inode));
    }

    // ��ʼ��system_openfiles
    for (auto &system_openfile: system_openfiles) {
        system_openfile.i_count = 0;
        system_openfile.fcb.d_index = 0;
    }

    // ��ʼ��pwds
    // ��ʵ�Ƕ���һ����
    fseek(disk, 0, SEEK_SET);
    fread(&pwds, sizeof(PWD), PWDNUM, disk);

    // ��ʼ��user_openfiles
    user_openfiles.clear();


    // ��ȡrootĿ¼��cur_dir��ǰĿ¼
    // ����һ��i�ڵ�
    // ��ʼ��cur_dir_inode
    cur_dir_inode = iget(1);
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for (i = 0; i < block_num; i++) {
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DATASTART + id * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&root) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&cur_dir) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&root) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&cur_dir) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);

}

// ��ʽ��
void format(){
    // admin�˻�
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // һ����ͨ�˻�
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // ���
    for(int i = 2; i < PWDNUM; i++){
        strcpy(pwds[0].password, "");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);

    // ���ó�����
    // ��ʱֻ��root��Ӧ�Ĵ���i�ڵ�����ݿ�
    // ռһ������i�ڵ��һ�����ݿ飬��Ŷ�Ϊ1
    file_system.s_block_size = FILEBLK - 1;
    file_system.s_dinode_size = BLOCKSIZ / DINODESIZ * DINODEBLK;
    file_system.s_free_dinode_num = file_system.s_dinode_size - 1;
    for(int i = 0; i < NICINOD; i++){
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = FILEBLK - 1;
    for(int i = FILEBLK; i > 1; i--){
        bfree(i);
    }
    file_system.s_fmod = '0';

    // ���ϵͳ�򿪱�
    for(auto & system_openfile : system_openfiles){
        system_openfile.i_count = 0;
        system_openfile.fcb.d_index = 0;
    }

    // ����û��򿪱�
    user_openfiles.clear();

    // ����ڴ��㻺��
    for(int i = 0; i < NHINO; i++){
        while(hinodes[i]->i_forw){
            iput(hinodes[i]->i_forw);
        }
    }

    // ��ǰĿ¼��Ϊroot
    cur_dir_inode = iget(1);
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for (i = 0; i < block_num; i++) {
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DATASTART + id * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&cur_dir) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&cur_dir) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);
}

// �ػ�������
void halt(){
    // ��鵱ǰ�û����ͷ�
    for(const auto& data: user_openfiles){
        user_open_table* tmp = data.second;
        for(int i = 0; i < NOFILE; i++){
            if(tmp->items[i].f_inode == nullptr)
                continue;
            int id = tmp->items[i].index_to_sysopen;
            system_openfiles[id].i_count--;
            if(system_openfiles[id].i_count == 0)
                system_openfiles[id].fcb.d_index = 0;
        }
    }
    user_openfiles.clear();

    // ��ջ���
    for(int i = 0; i < NHINO; i++){
        while(hinodes[i]->i_forw){
            iput(hinodes[i]->i_forw);
        }
    }

    // ���泬����
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);
    // �رմ���
    fclose(disk);
}

int login(const string &pwd) {
    int i;
    //����Ƿ���ƥ���PWD
    for (i = 0; i < PWDNUM && strcmp(pwd.c_str(), pwds[i].password) != 0; i++);
    if (i == PWDNUM)
        return -1;

    //����Ƿ��Ѿ���¼
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        return -2;
    }

    //���п�λ
    if (user_openfiles.size() < USERNUM) {
        auto *openTable = new user_open_table;
        openTable->p_uid = pwds[i].p_uid;
        openTable->p_gid = pwds[i].p_gid;
        memset(openTable->items, 0, sizeof(user_open_item) * NOFILE);
        user_openfiles.insert(pair<string, user_open_table *>(pwd, openTable));
        //���õ�ǰ�ļ�Ϊ��Ŀ¼
        cur_user = pwd;
        cur_dir_inode = iget(1);
        cur_dir = get_dir(1);
        return 0;
    }
    return -3;
}

void logout(const string &pwd) {
    user_open_table *u = user_openfiles.find(pwd)->second;
    //�ر�ÿ���ļ�
    for (auto &item: u->items) {
        unsigned short id_to_sysopen = item.index_to_sysopen;
        if ( id_to_sysopen!= -1) {
            //�رպ����Ϊ����Ҫ�ͷ��ڴ�ڵ�
            system_openfiles[id_to_sysopen].i_count--;
            if(system_openfiles[id_to_sysopen].i_count == 0){
                iput(iget(system_openfiles[id_to_sysopen].fcb.d_index));
            }
        }
    }
    free(u);
    user_openfiles.erase(pwd);
    cur_user = "";
}

bool access(int operation, inode *file_inode) {
    if (user_openfiles.find(cur_user) == user_openfiles.end()) {
        return false;//û�ҵ����û�
    }
    user_open_table *T = user_openfiles.find(cur_user)->second;
    bool creator = file_inode->dinode.di_uid == T->p_uid;//�ļ���uid�����û���uid ˵���Ǵ�����
    bool group = file_inode->dinode.di_gid == T->p_gid;//�ļ���gid�����û���gid ˵�������ڳ�Ա
    if(file_inode->dinode.di_gid==0&&operation==DELETE)//��Ŀ¼����ɾ
        return false;
    if(creator || group && READ)
        return true;
    return false;
}

//�жϻ����Ϸ��ԣ����ڣ����ȣ�λ�ڸ�Ŀ¼
//�������Ϸ�����
//������޸�.(�ݶ���


/* Ŀ¼�ļ��������Ƿ��п��У��Ƿ��ڸ�Ŀ¼�����������Ѵ����ļ�(findfile()ʵ��)���п���
 �������i��㣬��ʼ������i��㣬����i���д�ش��̣���Ŀ¼������д�ش��̡�
 �������i�ڵ�δʵ��*/
//inode* createFile(string pathname, unsigned short di_mode){
//    if(cur_user == "")
//        return nullptr;
//
//    FCB  fcb;
//    int i, j;
//    //�����ļ�
//    fcb = addFile(pathname);
//    //�ļ��Ѿ�����
//    if (fcb.d_index != 0)
//        return NULL;
//    else
//    {
//        struct inode* inode = ialloc(*this);
//        //��Ŀ¼�м���
//        ;
//        inode->dinode.di_mode = 1;
//        inode->dinode.di_uid = user_openfiles.find(cur_user)->second->p_uid;
//        inode->dinode.di_gid = user_openfiles.find(cur_user)->second->p_gid;
//        inode->dinode.di_size = 0;
//        inode->ifChange = 1;                //��Ҫ����д��
//        return j;
//    }
//}
struct dir get_dir(unsigned int d_index) {
    inode *dir_inode = iget(d_index);
    // �Ӵ��̼���Ŀ¼
    struct dir work_dir{};
    work_dir.size = dir_inode->dinode.di_size / DIRSIZ ;
    int i = 0;
    for (i = 0; i < work_dir.size / (BLOCKSIZ / DIRSIZ ); i++) {
        fseek(disk, DATASTART + BLOCKSIZ * dir_inode->dinode.di_addr[i], SEEK_SET);
        fread(&work_dir.files[BLOCKSIZ / DIRSIZ * i], 1, BLOCKSIZ, disk);
    }
    fseek(disk, DATASTART + BLOCKSIZ * dir_inode->dinode.di_addr[i], SEEK_SET);
    fread(&work_dir.files[BLOCKSIZ / DIRSIZ * i], 1, dir_inode->dinode.di_size % BLOCKSIZ, disk);
    return work_dir;
}
//���ļ�
int open_file(string pathname,int operation){
    if(!is_file(pathname))
        return false;                                               //�����ļ���ʽ�����ش�����
    inode *catalog;
    string filename;
    if(pathname.find_last_of('/')==string::npos){//��ǰĿ¼�����ļ�     ����·��
        catalog = cur_dir_inode;
        filename = pathname;
    }
    else{
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);//��ȡĿ¼�ļ����ڴ������ڵ�
    }
    if(access(READ,catalog))
        return -1;                                                  //Ȩ�޲��㣬���ش�����
    struct dir catalog_dir = get_dir(catalog->d_index);
    int file_index = -1;//�ļ���Ӳ��i���id
    int leisure = -1;//Ŀ¼�µĿ�������
    for (int i = 0;i<DIRNUM;i++) {
        if (catalog_dir.files[i].d_name == filename){//���ҳɹ�
            //���ҳɹ�����ȡ����������
            file_index = catalog_dir.files[i].d_index;
        }
        if(catalog_dir.files[i].d_index==0)
            leisure = i;
    }
    if(file_index==-1){//û���ҳɹ�
        if(operation==BUILD_OPEN)//������Ǵ����򿪣��ͷ��ش����룬δ�ҵ��ļ�
            return -1;
        else{//�Ǵ�����                      
            if(leisure==-1)                                             //��Ŀ¼�������򷵻ش�����
                return -1;
            //�����½��
            file_index = ialloc(1);
            inode *new_inode = iget(file_index);
            new_inode->dinode.di_mode = DIFILE;
            new_inode->ifChange = 1;
            //�޸�Ŀ¼������
            strcpy(catalog_dir.files[leisure].d_name,filename.data());
            catalog_dir.files[leisure].d_index = file_index;
            catalog_dir.size++;
            catalog->ifChange = 1;
            //д���ļ�����i������ݣ�д��Ŀ¼����i�������
            iput(new_inode);
            iput(catalog);
        }
    }
    //�޸�ϵͳ���ļ���
    int sys_leisure = 0;    
    for(;sys_leisure< SYSOPENFILE;sys_leisure++){//�ҵ�����
        if(system_openfiles[sys_leisure].i_count==0){
            system_openfiles[sys_leisure].i_count++;
            system_openfiles[sys_leisure].fcb.d_index = file_index;
            strcpy(system_openfiles[sys_leisure].fcb.d_name,filename.data());
            break;
        }
    }
    if(sys_leisure == SYSOPENFILE)
        return -1;//û�ҵ�ϵͳ�򿪱���еı���
    //�޸��û��ļ��򿪱�
    user_open_table *T = user_openfiles.find(cur_user)->second;
    int usr_leisure = 0;
    for(;usr_leisure<SYSOPENFILE;usr_leisure++){
        if(T->items[usr_leisure].f_count==0){
            T->items[usr_leisure].f_count++;
            if(operation==FP_TAIL_OPEN)
                T->items[usr_leisure].f_offset=iget(file_index)->dinode.di_size;
            else
                T->items[usr_leisure].f_offset=0;
            T->items[usr_leisure].index_to_sysopen=sys_leisure;
            T->items[usr_leisure].u_default_mode=BUILD_OPEN;
            return usr_leisure;//�����û��򿪱�����
        }
    }
    return -1;//û�ҵ��û��򿪱���б���
}
// �����ļ��У��������ļ�·��
int mkdir(string &pathname) {
    if (!judge_path(pathname)) {
        return false;
    } else {
        inode *catalog;
        struct dir catalog_dir;
        string filename;
        if(pathname.find_last_of('/')==std::string::npos){
            catalog = cur_dir_inode;
            catalog_dir = cur_dir;
            filename = pathname;
        }
        else{
            int pos = pathname.find_last_of('/') + 1;
            string father_path = pathname.substr(0, pos - 1);
            filename = pathname.substr(pos);
            catalog = find_file(father_path);
            if (catalog == nullptr) {
                return -1;//�޸�·�������ش�����
            }
            catalog_dir = get_dir(catalog->d_index);
        }
        if (access(CHANGE, catalog))
            return -1;//Ȩ�޲��㣬���ش�����
        //�ж�Ŀ¼�ļ��������Ƿ��п���
        //С�����Ŀ¼����˵������
        if (catalog_dir.size < DIRNUM) {
            //�ж��Ƿ��ظ�
            int leisure;
            for (int i = 0;i<DIRNUM ;i++ ){
                if (catalog_dir.files[i].d_name == file) //������Ѿ����ڵ��ļ��У��򷵻ش�����
                    return -1;
                if (catalog_dir.files[i].d_index==0)
                    leisure = i;
            }
            //������������Ӳ��������
            int new_d_index = ialloc(1);
            inode *new_inode = iget(new_d_index);
            int block_amount = sizeof(dir) / BLOCKSIZ + 1;
            for (int j = 0; j < block_amount; j++) {
                new_inode->dinode.di_addr[j] = balloc();
            }
            new_inode->dinode.di_mode = DIDIR;
            new_inode->dinode.di_size = sizeof(dir);
            //��ʼ��Ӳ��������(�����������ialloc�г�ʼ��)
            struct dir new_dir = get_dir(new_d_index);
            strcpy(new_dir.files[0].d_name,"root");
            new_dir.files[0].d_index = 1;
            new_dir.size = 0;
            //�ҵ���Ŀ¼���е�Ŀ¼��,д���ļ������ļ����̽��
            int leisure = seek_catalog_leisure();
            strcpy(catalog_dir.files[leisure].d_name,filename.data());
            catalog_dir.files[leisure].d_index = new_d_index;
            catalog_dir.size++;
            catalog->ifChange = 1;
            new_inode->ifChange = 1;
            //����Ŀ¼���ڴ�i���д�����i��㣬�����ļ��е��ڴ�i���д�����i���
            iput(catalog);
            iput(new_inode);
            return 1;
        } else {//û�п��У�ʧ��
            return -1;
        }
    }
}


int rmdir(const string& pathname) {
    if (!judge_path(pathname)) {
        return -1;
    } else {
        inode *father_catalog;
        string filename;
        if(pathname.find_last_of('/')==std::string::npos){
            father_catalog = cur_dir_inode;
            filename = pathname;
        }
        else{
            int pos = pathname.find_last_of('/') + 1;
            string father_path = pathname.substr(0, pos - 1);
            father_catalog = find_file(father_path);
            if(father_catalog == nullptr)
                return -1;//�޸�·�������ش�����
            filename = pathname.substr(pos);
        }
        inode *catalog = find_file(pathname);
        if (access(CHANGE, father_catalog))
            return -1;//Ȩ�޲��㣬���ش�����
        if (catalog == nullptr) {
            return -1;//�޸�·�������ش�����
        }
        struct dir catalog_dir = get_dir(catalog->d_index);//�õ�·����Ŀ¼dir����
        struct dir father_dir = get_dir(father_catalog->d_index);//�õ���Ŀ¼��dir����
        if (catalog_dir.size != 0) {
            return -1;//��·����Ŀ¼�����ݣ�ʧ�ܡ�
        } else {
            //����Ŀ¼���������ɾ��
            for (auto &i: father_dir.files) {
                if (i.d_index == catalog->d_index) {//���Ҹ��ļ����±�
                    i.d_index = 0;
                    memset(i.d_name, 0, DIRSIZ);
                    father_dir.size--;
                    catalog->ifChange = 1;
                    father_catalog->ifChange = 1;
                    catalog->dinode.di_number--;
                    break;
                }
            }
            //����Ŀ¼����i���ɾ�����ͷŸý����ָ��������
            iput(catalog);
            //����Ŀ¼����д�ش���������
            iput(father_catalog);
            return 1;
        }
    }
}

//�ƶ�ϵͳ��ǰ·��
int chdir(const string& pathname) {
    if (!judge_path(pathname)) {
        return -1;
    } else {
        inode *catalog = find_file(pathname);
        if (access(CHANGE, catalog))
            return -1;//Ȩ�޲��㣬���ش�����
        if (catalog == nullptr) {
            return -1;//�޸�·�������ش�����
        }
        cur_dir = get_dir(catalog->d_index);
        cur_dir_inode = catalog;
    }
    return 1;
}

int show_dir() {
    if (access(READ, cur_dir_inode))
        return -1;//Ȩ�޲��㣬���ش�����
    for (auto & file : cur_dir.files) {
        if (file.d_index != 0) {
            cout << file.d_name << endl;//�����ǰ·���µ��ļ�����
        }
    }
    return 0;
}

//��ʾ��ǰ�û�ss
string whoami() {
    return cur_user;
}




// �ر�һ���Ѿ����û����˵��ļ�

void closeFile(const string& pathname) {
    // �ж��ļ����Ƿ�Ϸ�
    if (judge_path(pathname)!=2) {
        return;
    }
    // ��ȡ�û��Ĵ򿪱�
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // ��ȡ�û�uid


    // ������ѯ���ļ�
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
    // �û�û�д򿪸��ļ�
    if (!found)
        return;

    // ȷ������
    // �ͷŸ��ļ����ڴ�i�ڵ�
    iput(userOpenTable->items[i].f_inode);
    userOpenTable->items[i].f_inode = nullptr;
    // ϵͳ�򿪱��ж��Ƿ���Ҫ�ر�
    system_openfiles[id].i_count--;
    if (system_openfiles[id].i_count == 0) {
        // ������ļ���ϵͳ�򿪱��йر�
        system_openfiles[id].fcb.d_index = 0;
    }
}

// ���û��Դ򿪵��ļ��ж�ȡ����
// ���ַ���ʽ��������
// û��ʵ��Ȩ���ж�
string readFile(string pathname) {
    // �ж��ļ����Ƿ�Ϸ�
    if (judge_path(pathname)!=2) {
        return {};
    }

    // �ж��û��Ը��ļ��Ƿ��ж�Ȩ��
    // if(access())

    // �ж��ļ��Ƿ��û���
    // ��ȡ�û��Ĵ򿪱�
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // ��ȡ�û�uid

    // ������ѯ���ļ�
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
    // �û�û�д򿪸��ļ�
    if (!found)
        return {};

    // ȷ������
    unsigned int dinode_id = system_openfiles[id].fcb.d_index;
    // �����������i�ڵ�
    auto *pdinode = (struct dinode *) malloc(sizeof(struct dinode));
    long addr = DINODESTART + dinode_id * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fread(pdinode, DINODESIZ, 1, disk);
    // ׼����ȡ����
    unsigned short size = pdinode->di_size;
    int block_num = size / BLOCKSIZ;
    char *res = (char *) malloc(size);
    for (i = 0; i < block_num; i++) {
        addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread(res + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    addr = DATASTART + pdinode->di_addr[i] * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread(res + i * BLOCKSIZ, size - block_num * BLOCKSIZ, 1, disk);
    return res;
}

inode *find_file(string addr) {
    string Subaddr;
    unsigned int index;//��������ļ����ҵ���FCB��d_index
    int isInDir = 0;//�ж��Ƿ���Ŀ¼��
    int first;//��һ�γ���'/'��λ��
    if (addr.empty())
        return nullptr;
    if (addr[addr.length() - 1] == '/')
        return nullptr;
    if (addr[0] == '/') {//����·��
        addr = addr.substr(1, addr.length());
        index = cur_dir.files[0].d_index;
        cur_dir = get_dir(index);
    }

    //���β��Ҹ���Ŀ¼�ڵ���һ��Ŀ¼
    while (addr.find_first_of('/') != string::npos) {
        first = addr.find_first_of('/');
        Subaddr = addr.substr(0, first);
        addr = addr.substr(first + 1, addr.length());
        for (auto &file: cur_dir.files) {
            if (strcmp(file.d_name, Subaddr.c_str()) == 0) {
                index = file.d_index;
                cur_dir = get_dir(index);
                isInDir = 1;
                break;
            }
        }
        if (isInDir == 0)
            return nullptr;
        isInDir = 0;
    }

    //�õ������ļ����ڴ�i���ָ��
    for (auto &file: cur_dir.files) {
        if (strcmp(file.d_name, addr.c_str()) == 0)
            return iget(file.d_index);
    }
    return nullptr;

}

// Ȩ��δʵ��
//bool writeFile(const string& pathname, int write_mode,const string& content) {
//    // �ж��ļ����Ƿ�Ϸ�
//    if (!is_file(pathname)) {
//        return {};
//    }
    // �ж��û��Ը��ļ��Ƿ���дȨ��
    // if(access())

    // �ж��ļ��Ƿ��û���
    // ��ȡ�û��Ĵ򿪱�
//    user_open_table *userOpenTable = user_openfiles[cur_user];
//    // ��ȡ�û�uid
//    unsigned short p_uid = userOpenTable->p_uid;
//
//    // ������ѯ���ļ�
//    unsigned short id;
//    bool found = false;
//    int i;
//    for (i = 0; i < NOFILE; i++) {
//        if (userOpenTable->items[i].f_inode == nullptr) {
//            continue;
//        }
//        id = userOpenTable->items[i].index_to_sysopen;
//        if (!strcmp(system_openfiles[id].fcb.d_name, pathname.c_str()) && system_openfiles[id].i_count != 0) {
//            found = true;
//            break;
//        }
//    }
//    // �û�û�д򿪸��ļ�
//    if (!found)
//        return false;
//
//    // ����ģʽ
//}