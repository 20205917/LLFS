//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include "RunningSystem.h"

void initial() {
    // ��Ӳ��
    disk = fopen(disk_file_name, "wb+");
    char nothing[BLOCKSIZ] = {0};
    fseek(disk, 0, SEEK_SET);
    for(int i = 0; i < FILEBLK + DINODEBLK + 2; i++)
        fwrite(nothing, BLOCKSIZ, 1, disk);

    // ��ʼ��������
    // ��ʱֻ��root��Ӧ�Ĵ���i�ڵ�����ݿ�
    // ռһ������i�ڵ��һ�����ݿ飬��Ŷ�Ϊ1
    file_system.s_block_size = FILEBLK - 1;
    file_system.s_dinode_size = BLOCKSIZ / DINODESIZ * DINODEBLK;
    file_system.s_free_dinode_num = file_system.s_dinode_size - 1;
    for (int i = 0; i < NICINOD; i++) {
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = 0;
    file_system.s_pfree_block = 0;
    for (int i = FILEBLK; i > 1; i--) {
        bfree(i);
    }
    file_system.s_fmod = '0';

    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);

    // ��ʼ��root����i�ڵ�
    cur_dir_inode = (hinode) malloc(sizeof(struct inode));
    cur_dir_inode->dinode.di_number = 1;
    cur_dir_inode->dinode.di_mode = DIDIR;
    cur_dir_inode->dinode.di_uid = 0;
    cur_dir_inode->dinode.di_gid = 0;
    cur_dir_inode->dinode.di_size = sizeof(struct dir);
    cur_dir_inode->dinode.di_addr[0] = 0;

    long addr = DINODESTART + 1 * DINODESIZ;
    fseek(disk, addr, SEEK_SET);
    fwrite(&(cur_dir_inode->dinode), DINODESIZ, 1, disk);

    struct dir root;
    // ��ʼ��rootĿ¼���ݿ�����
    root.size = 2;
    // ��Ŀ¼
    root.files[0].d_index = 1;
    strcpy(root.files[0].d_name, "root");
    // ��Ŀ¼
    root.files[1].d_index = 1;
    strcpy(root.files[1].d_name, "root");
    for (int i = 2; i < DIRNUM; i++) {
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
    for (int i = 2; i < PWDNUM; i++) {
        pwds[i].p_uid = 0;
        strcpy(pwds[i].password, "");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
    fclose(disk);
}

void install() {
    // ��Ӳ��
    disk = fopen(disk_file_name, "rb+");

    // ��ʼ��file_system
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fread(&file_system, sizeof(file_system), 1, disk);

    // ��ʼ��hinodes
    for (auto &i: hinodes) {
        i = (hinode) malloc(sizeof(struct inode));
        memset(i, 0, sizeof(inode));
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
    cur_path = "root";
    int size = cur_dir_inode->dinode.di_size;
    int block_num = size / BLOCKSIZ;
    unsigned int id;
    long addr;
    int i;
    for (i = 0; i < block_num; i++) {
        id = cur_dir_inode->dinode.di_addr[i];
        addr = DATASTART + id * BLOCKSIZ;
        fseek(disk, addr, SEEK_SET);
        fread((char *) (&cur_dir_inode->content) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    cur_dir_inode->content = malloc(size - BLOCKSIZ * block_num);
    fread((char *) (cur_dir_inode->content) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);

}

// ��ʽ��
void format() {
    // admin�˻�
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // һ����ͨ�˻�
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // ���
    for (int i = 2; i < PWDNUM; i++) {
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
    for (int i = 0; i < NICINOD; i++) {
        file_system.s_dinodes[i] = i + 2;
    }
    file_system.s_pdinode = 0;
    file_system.s_rdinode = 2;

    file_system.s_free_block_size = 0;
    file_system.s_pfree_block = 0;
    for (int i = FILEBLK; i > 1; i--) {
        bfree(i);
    }
    file_system.s_fmod = '0';

    // ���ϵͳ�򿪱�
    for (auto &system_openfile: system_openfiles) {
        system_openfile.i_count = 0;
        system_openfile.fcb.d_index = 0;
    }

    // ����û��򿪱�
    user_openfiles.clear();

    // ����ڴ��㻺��
    for (int i = 0; i < NHINO; i++) {
        while (hinodes[i]->i_forw) {
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
        fread((char *) (&cur_dir_inode->content) + i * BLOCKSIZ, BLOCKSIZ, 1, disk);
    }
    id = cur_dir_inode->dinode.di_addr[block_num];
    addr = DATASTART + id * BLOCKSIZ;
    fseek(disk, addr, SEEK_SET);
    fread((char *) (&cur_dir_inode->content) + block_num * BLOCKSIZ, size - BLOCKSIZ * block_num, 1, disk);
    // admin�˻�
    pwds[0].p_uid = 0;
    pwds[0].p_gid = 0;
    strcpy(pwds[0].password, "admin");
    // һ����ͨ�˻�
    pwds[1].p_uid = 1;
    pwds[1].p_gid = 1;
    strcpy(pwds[1].password, "1");
    // ���
    for (int j = 2; j < PWDNUM; j++) {
        pwds[j].p_uid = 0;
        strcpy(pwds[j].password, "");
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
}

// �ػ�������
void halt() {
    // ��鵱ǰ�û����ͷ�
    for (const auto &data: user_openfiles) {
        user_open_table *tmp = data.second;
        for (int i = 0; i < NOFILE; i++) {
            if (tmp->items[i].f_inode == nullptr)
                continue;
            int id = tmp->items[i].index_to_sysopen;
            system_openfiles[id].i_count--;
            if (system_openfiles[id].i_count == 0)
                system_openfiles[id].fcb.d_index = 0;
        }
    }
    user_openfiles.clear();

    // ��ջ���
    for (int i = 0; i < NHINO; i++) {
        while (hinodes[i]->i_forw) {
            iput(hinodes[i]->i_forw);
        }
    }

    // ���泬����
    fseek(disk, BLOCKSIZ, SEEK_SET);
    fwrite(&file_system, sizeof(struct super_block), 1, disk);

    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
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
    if(!strcmp(pwd.c_str(), cur_user.c_str()))
        return -2;

    //
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        cur_user = pwd;
        return -3;
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
        return 0;
    }
    return -3;
}

void logout(const string &pwd) {
    user_open_table *u = user_openfiles.find(pwd)->second;
    //�ر�ÿ���ļ�
    for (auto &item: u->items) {
        short id_to_sysopen = item.index_to_sysopen;
        if (id_to_sysopen != -1) {
            //�رպ����Ϊ����Ҫ�ͷ��ڴ�ڵ�
            system_openfiles[id_to_sysopen].i_count--;
            if (system_openfiles[id_to_sysopen].i_count == 0) {
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
    if (file_inode->dinode.di_gid == 0 && operation == FDELETE)//��Ŀ¼����ɾ
        return false;
    if (creator || group && READ)
        return true;
    return false;
}

//�жϻ����Ϸ��ԣ����ڣ����ȣ�λ�ڸ�Ŀ¼
//�������Ϸ�����
//������޸�.(�ݶ���


//���ļ�
int open_file(string &pathname, int operation) {
    if (judge_path(pathname) != 2)
        return -1;                                               //�����ļ���ʽ�����ش�����
    inode *catalog;
    string filename;
    if (pathname.find_last_of('/') == string::npos) {//��ǰĿ¼�����ļ�     ����·��
        catalog = cur_dir_inode;
        filename = pathname;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);//��ȡĿ¼�ļ����ڴ������ڵ�
        if (catalog == nullptr) {
            return -1;//�޸�·�������ش�����
        }
    }
    if (!access(READ, catalog))
        return -1;                                                  //Ȩ�޲��㣬���ش�����
    auto *catalog_dir = (dir *) catalog->content;
    unsigned int file_index;//�ļ���Ӳ��i���id
    int leisure = -1;//Ŀ¼�µĿ�������
    int i;
    for (i = 0; i < DIRNUM; i++) {
        if (catalog_dir->files[i].d_name == filename) {//���ҳɹ�
            //���ҳɹ�����ȡ����������
            file_index = catalog_dir->files[i].d_index;
            break;
        }
        if (catalog_dir->files[i].d_index == 0)
            leisure = i;
    }
    inode * new_inode;
    if (i == DIRNUM) {//û���ҳɹ�
        if (operation != BUILD_OPEN)//������Ǵ����򿪣��ͷ��ش����룬δ�ҵ��ļ�
            return -2;
        else {//�Ǵ�����
            if (leisure == -1)                                             //��Ŀ¼�������򷵻ش�����
                return -3;
            //�����½��
            file_index = ialloc(1);
            new_inode = iget(file_index);
            new_inode->dinode.di_mode = DIFILE;
            new_inode->ifChange = 1;
            //�޸�Ŀ¼������
            strcpy(catalog_dir->files[leisure].d_name, filename.data());
            catalog_dir->files[leisure].d_index = file_index;
            catalog_dir->size++;
            catalog->ifChange = 1;
            //д���ļ�����i������ݣ�д��Ŀ¼����i�������
        }
    }
    else// ���ҳɹ����ҵ��ڴ������ڵ�
        new_inode = iget(file_index);
    //�޸�ϵͳ���ļ���
    short sys_leisure = 0;
    for (; sys_leisure < SYSOPENFILE; sys_leisure++) {//�ҵ�����
        if (system_openfiles[sys_leisure].i_count == 0) {
            system_openfiles[sys_leisure].i_count++;
            system_openfiles[sys_leisure].fcb.d_index = file_index;
            strcpy(system_openfiles[sys_leisure].fcb.d_name, filename.data());
            break;
        }
    }
    if (sys_leisure == SYSOPENFILE)
        return -4;//û�ҵ�ϵͳ�򿪱���еı���
    //�޸��û��ļ��򿪱�
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
            return usr_leisure;//�����û��򿪱�����
        }
    }
    return -5;//û�ҵ��û��򿪱���б���
}

//�ر��ļ�
int close_file(int fd) {
    user_open_table *T = user_openfiles.find(cur_user)->second;
    T->items[fd].f_count--;//�û��򿪱������-1
    system_openfiles[T->items[fd].index_to_sysopen].i_count--;
    return 1;
}

// �����ļ��У��������ļ�·��
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
            return -1;//�޸�·�������ش�����
        }
    }
    if (!access(CHANGE, catalog))
        return -1;//Ȩ�޲��㣬���ش�����
    //�ж�Ŀ¼�ļ��������Ƿ��п���
    //С�����Ŀ¼����˵������
    dir *catalog_dir = (dir *) catalog->content;
    if (catalog_dir->size < DIRNUM) {
        //�ж��Ƿ��ظ�
        int leisure;
        for (int i = 0; i < DIRNUM; i++) {
            if (catalog_dir->files[i].d_name == filename) //������Ѿ����ڵ��ļ��У��򷵻ش�����
                return -1;
            if (catalog_dir->files[i].d_index == 0)
                leisure = i;
        }


        //������������Ӳ��������
        int new_d_index = ialloc(1);
        inode *new_inode = iget(new_d_index);
        new_inode->dinode.di_addr[0] = balloc();
        new_inode->dinode.di_mode = DIDIR;
        new_inode->dinode.di_size = sizeof(dir);
        new_inode->ifChange = 1;


        //��ʼ��Ӳ��������(�����������ialloc�г�ʼ��)
        new_inode->content = malloc(sizeof(dir));
        memset(new_inode->content, 0, sizeof(dir));
        strcpy(((dir *) new_inode->content)->files[0].d_name, "root");
        ((dir *) new_inode->content)->files[0].d_index = 1;
        strcpy(((dir *) new_inode->content)->files[1].d_name,
               father_path.substr(0, father_path.find_last_of('/') - 1).c_str());
        ((dir *) new_inode->content)->files[1].d_index = catalog->d_index;
        ((dir *) new_inode->content)->size = 2;
        //�ҵ���Ŀ¼���е�Ŀ¼��,д���ļ������ļ����̽��
        strcpy(catalog_dir->files[leisure].d_name, filename.data());
        catalog_dir->files[leisure].d_index = new_d_index;
        catalog_dir->size++;
        catalog->ifChange = 1;
        return 1;
    }

    return -1;
}
//Ӳ����
int hard_link(string &pathname,string &newname){
    string father_path;
    inode *filea;
    string filename;
    if (pathname[0] == '/')
        pathname = "root" + pathname;
    filea = find_file(pathname);
    if (filea == nullptr)
            return -1;//�޸�·�������ش�����
    if (!access(READ, filea))
        return -1;//Ȩ�޲��㣬���ش�����
    inode * catalog_b = cur_dir_inode;
    if(((dir*)catalog_b->content)->size==DIRNUM)
        return -1;//Ŀ¼�޿���
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
            return -1;//�޸�·�������ش�����
        filename = pathname.substr(pos);
    }
    inode *catalog = find_file(pathname);
    if (catalog == nullptr)
            return -1;//�޸�·�������ش�����
    if (!access(CHANGE, father_catalog))
        return -1;//Ȩ�޲��㣬���ش�����
    auto *catalog_dir = (dir *) catalog->content;//�õ�·����Ŀ¼dir����
    auto *father_dir = (dir *) father_catalog->content;//�õ���Ŀ¼��dir����
    if (catalog_dir->size > 2) {
        return -1;//��·����Ŀ¼�����ݣ�ʧ�ܡ�
    } else {
        //����Ŀ¼���������ɾ��
        for (auto &i: father_dir->files) {
            if (i.d_index == catalog->d_index) {//���Ҹ��ļ����±�
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

//�ƶ�ϵͳ��ǰ·��
int chdir(string &pathname) {
    int value=0;//�ж��Ƿ��Ǿ���·��
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
        else if(pathname[0]=='r'&&pathname[1]=='o'&&pathname[2]=='o'&&pathname[3]=='t')
            value = 1;
        inode *catalog = find_file(pathname);
        if (catalog == nullptr) {
            return -2;//�޸�·�������ش�����
        }
        if (!access(CHANGE, catalog))
            return -3;//Ȩ�޲��㣬���ش�����
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
        return -1;//Ȩ�޲��㣬���ش�����
//    for (auto &file: ((dir *) cur_dir_inode->content)->files) {
//        if (file.d_index != 0) {
//            cout << file.d_name << endl;//�����ǰ·���µ��ļ�����
//        }
//    }
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
    // ��root��ʼ
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
            inMemory = true;
            tmp = findHinode(id);
            if(tmp == nullptr){
                inMemory = false;
                tmp = getDinodeFromDisk(id);
            }
            if(tmp->dinode.di_mode == DIDIR){
                std::cout << "<DIR>  ";
                std::cout << dirs->files[i].d_name << std::endl;
                show_dir_tree(dirs->files[i].d_index, depth + 1);
            }
            else{
                std::cout << "<FILE> ";
                std::cout << dirs->files[i].d_name << std::endl;
            }
            size--;
        }
    }
    if(!inMemory){
        free(tmp);
    }
    return 0;
}

//��ʾ��ǰ�û�ss
string whoami() {
    return cur_user;
}


/* �ļ����Ƿ�Ϸ�����Ҫfindfile���ж��Ƿ���Ȩ��
// �޸ĸ�Ŀ¼��������д����̣�iput()ɾ���ļ�
// falseɾ��ʧ�� trueɾ���ɹ�
// Ȩ��δʵ�� iputδʵ��*/
int deleteFile(string pathname) {
    inode *catalog;
    string filename;
    if (pathname.find_last_of('/') == string::npos) {//��ǰĿ¼�����ļ�     ����·��
        catalog = cur_dir_inode;
        filename = pathname;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);//��ȡĿ¼�ļ����ڴ������ڵ�
        if (catalog == nullptr)
            return -2;//�޸�·�������ش�����
    }
    if (!access(READ, catalog))
        return PERMISSION_DD;                                                  //Ȩ�޲��㣬���ش�����
    auto *catalog_dir = (dir *) catalog->content;
    unsigned int file_index;//�ļ���Ӳ��i���id
    int i;
    for (i = 0; i < DIRNUM; i++) {
        if (catalog_dir->files[i].d_name == filename) {//���ҳɹ�
            //���ҳɹ�����ȡ����������
            file_index = catalog_dir->files[i].d_index;
            break;
        }
    }
    inode *file_inode;
    if (i == DIRNUM) {//û���ҳɹ�
        return NOT_FOUND;//�޸��ļ���ɾ��ʧ�ܣ����ش�����
    }
    //���ҳɹ����ҵ��ڴ������ڵ�
    file_inode = iget(file_index);
    //�޸�ϵͳ���ļ���
    for (short sys_i = 0; sys_i < SYSOPENFILE; sys_i++) {//��ϵͳ�򿪱�ı���
        if (system_openfiles[sys_i].fcb.d_index == file_index && system_openfiles[sys_i].i_count != 0)
            return -1;//���ļ����ڱ�ϵͳ��
    }
    //ɾ���ļ������ļ�Ӳ���Ӵ���Ϊ0�����ͷŽ������ڵ�������ָ����Ϊ��
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
    return 0;//�ɹ�ɾ��
}

// �ر�һ���Ѿ����û����˵��ļ�

void closeFile(const string &pathname) {
    // �ж��ļ����Ƿ�Ϸ�
    if (judge_path(pathname) != 2) {
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
string readFile(int fd,int len) {
    //�ж��ļ��Ƿ��û���
    // ��ȡ�û��Ĵ򿪱�
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // ��ȡ�û�uid
    unsigned short p_uid = userOpenTable->p_uid;
    // ʹ��fd��ȡ���ļ�
    struct user_open_item opened_file = userOpenTable->items[fd];

    // Ϊ0˵����ȡ����
    if(opened_file.f_count == 0)
        return "";

    hinode file_inode = opened_file.f_inode;

    // �ж��û��Ը��ļ��Ƿ��ж�Ȩ��
    if(!access(Read, file_inode))
        return "";

    if(opened_file.f_offset+len > file_inode->dinode.di_size){
        len = file_inode->dinode.di_size - opened_file.f_offset;
    }

    //��ȡ
    string result = string((char*)file_inode->content+opened_file.f_offset,len);

    //�޸�offset
    opened_file.f_offset +=len;

    return result;
}

inode *find_file(string addr) {
    dir *temp_dir = (dir *) cur_dir_inode->content;   //����Ŀ¼���
    unsigned int index; //��������ļ����ҵ���FCB��d_index
    int isInDir = 0;    //�ж��Ƿ���Ŀ¼��
    hinode final = nullptr;//���Ա����˳�ѭ�������һ�����ļ���Ŀ¼���ڴ�i���
    if (addr[0] == '/') {//����·��
        addr = addr.substr(1, addr.length());
        temp_dir = (dir *) (iget(1)->content);
        index = 1;
    }
    char *ADDR = new char[addr.length() + 1];
    addr.copy(ADDR, addr.length(), 0); //addr���ݸ��Ƶ�ADDR[]
    *(ADDR + addr.length()) = '\0'; //ĩβ����'/0'
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
        if (isInDir == 0) {//�Ҳ������ϵ��ļ���
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

// Ȩ��δʵ��

int writeFile(int fd, const string& content) {

 //�ж��ļ��Ƿ��û���
 // ��ȡ�û��Ĵ򿪱�
    user_open_table *userOpenTable = user_openfiles[cur_user];
    // ��ȡ�û�uid
    unsigned short p_uid = userOpenTable->p_uid;
    // ʹ��fd��ȡ���ļ�
    struct user_open_item opened_file = userOpenTable->items[fd];
    // Ϊ0˵����ȡ����
    if(opened_file.f_count == 0)
        return -1; // �ļ���ʶ������

    hinode file_inode = opened_file.f_inode;
    file_inode->ifChange = 1;
    unsigned long offset = opened_file.f_offset;


    // д�ļ�
    std::string tmp((char*)file_inode->content);
    tmp = tmp.substr(0, offset);
    tmp += content;

    free(file_inode->content);
    file_inode->content = (char*) malloc(tmp.size() + 1);
    strcpy((char*)file_inode->content, tmp.c_str());

    file_inode->dinode.di_size = tmp.size() + 1;
    userOpenTable->items[fd].f_offset = tmp.size();
    return true;
}
int file_seek(int fd,int offset,int fseek_mode){
    user_open_table *T = user_openfiles.find(cur_user)->second;
    int cur_offset = T->items[fd].f_offset;
    int file_capacity = T->items[fd].f_inode->dinode.di_size;
    switch (fseek_mode)
    {
    case HEAD_FSEEK://��ͷ�ƶ�
        cur_offset = offset;
        break;
    case CUR_SEEK://�ӵ�ǰ�ƶ�
        cur_offset += offset;
        break;
    case LAST_SEEK://��ĩβ�ƶ�
        cur_offset = file_capacity + offset;
    default:
        cur_offset += offset;
        break;
    }
    if(cur_offset<0)
        return -1;//�ƶ�ƫ��������
    if(cur_offset>file_capacity){
        file_capacity = cur_offset + 1;
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
// Ӳ���Ӵ�����ʼ��Ϊ1
// ��Ҫ�����ļ�ƫ�������˴�δʵ��
// int createFile(string pathname, int operation){
int createFile(string pathname){
//    if (judge_path(pathname) != 2)
//        return -1;                                               //�����ļ���ʽ�����ش�����
    inode *catalog;
    string filename;
    if (pathname.find_last_of('/') == string::npos) {//��ǰĿ¼�����ļ�     ����·��
        catalog = cur_dir_inode;
        filename = pathname;
    } else {
        if (pathname[0] == '/')
            pathname = "root" + pathname;
        int pos = pathname.find_last_of('/') + 1;
        string father_path = pathname.substr(0, pos - 1);
        filename = pathname.substr(pos);
        catalog = find_file(father_path);//��ȡĿ¼�ļ����ڴ������ڵ�
        if (catalog == nullptr)
            return -1;//�޸�·�������ش�����
    }
    if (!access(Write, catalog))
        return -1;                                                  //Ȩ�޲��㣬���ش�����
    auto *catalog_dir = (dir *) catalog->content;
    unsigned int file_index;//�ļ���Ӳ��i���id
    int leisure = -1;//Ŀ¼�µĿ�������
    int i;
    for (i = 0; i < DIRNUM; i++) {
        if (catalog_dir->files[i].d_name == filename) {//���ҳɹ�
            //ֱ�ӷ���
            return -2;
        }
        if (catalog_dir->files[i].d_index == 0)
            leisure = i;
    }
    inode * new_inode;
    if (i == DIRNUM) {//û���ҳɹ�
        if (leisure == -1)                                             //��Ŀ¼�������򷵻ش�����
            return -3;
        //�����½��
        file_index = ialloc(1);
        new_inode = iget(file_index);
        new_inode->dinode.di_mode = DIFILE;
        new_inode->ifChange = 1;
        //
        new_inode->dinode.di_number = 1;
        //�޸�Ŀ¼������
        strcpy(catalog_dir->files[leisure].d_name, filename.data());
        catalog_dir->files[leisure].d_index = file_index;
        catalog_dir->size++;
        catalog->ifChange = 1;

    }
    return 0;//�����ɹ�
}

// չʾ�����û�
void show_all_users(){
    std::cout << "uid" << "     gid" << "     pwd";
    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, "")){
            std::cout << pwds[i].p_uid << "     "
            << pwds[i].p_gid << "     "
            << pwds[i].password << std::endl;
        }
    }
}

// չʾ���е�¼�û�
void show_login_users(){
    std::cout << "uid" << "     gid" << "     pwd";
    for(const auto& user: user_openfiles){
        for(int i = 0; i < USERNUM; i++){
            if(!strcmp(pwds[i].password, user.first.c_str())){
                std::cout << pwds[i].p_uid << "     "
                          << pwds[i].p_gid << "     "
                          << pwds[i].password << std::endl;
            }
        }
    }
}

int switch_user(const string& pwd){
    int i;
    //����Ƿ���ƥ���PWD
    for (i = 0; i < PWDNUM && strcmp(pwd.c_str(), pwds[i].password) != 0; i++);
    if (i == PWDNUM)
        return -1; // pwd��Ч

    //����Ƿ��Ѿ���¼
    if(!strcmp(pwd.c_str(), cur_user.c_str()))
        return -2; // pwd����ǰ�û�

    //
    if (user_openfiles.find(pwd) != user_openfiles.end()) {
        cur_user = pwd;
        return 0; // �л��û��ɹ�
    }
    return -1;
}

// �����û�������
int usermod(int uid, int gid){
    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, "")){
            continue;
        }
        if(uid == pwds[i].p_uid){
            pwds[i].p_gid = gid;
            return 0;
        }
    }
    return -1; // uid��Ч
}

// �����û�
int useradd(int gid, const std::string& pwd){
    int uid = 0;
    int index = 0;
    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, "")){
            index = i;
            continue;
        }
        if(uid <= pwds[i].p_uid){
            uid = pwds[i].p_uid + 1;
        }
    }
    if(index == 0){
        return -1; // �û��ﵽ����
    }
    pwds[index].p_uid = uid;
    pwds[index].p_gid = gid;
    strcpy(pwds[index].password, pwd.c_str());
    return 0; // ����û��ɹ�
}

int change_file_owner(string& pathname, int uid){
    inode* tmp =  find_file(pathname);
    if(tmp == nullptr)
        return -1; // ·����Ч
    int o_uid = tmp->dinode.di_uid;

    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, cur_user.c_str())){
            if(pwds[i].p_uid == 0){
                pwds[i].p_uid = uid;
                return 0; // ����Ա����ֱ�Ӹ�
            }
            else if(o_uid != pwds[i].p_uid){
                return -2; // �����Ǵ������޷��޸�
            }
            // ������Ҳ���Ը�
            pwds[i].p_uid = uid;
            return 0;
        }
    }
    return -1;
}

int change_file_group(string& pathname, int gid){
    inode* tmp =  find_file(pathname);
    if(tmp == nullptr)
        return -1; // ·����Ч
    int o_gid = tmp->dinode.di_gid;

    for(int i = 0; i < USERNUM; i++){
        if(!strcmp(pwds[i].password, cur_user.c_str())){
            if(pwds[i].p_uid == 0){
                pwds[i].p_gid = gid;
                return 0; // ����Ա����ֱ�Ӹ�
            }
            else if(o_gid != pwds[i].p_gid){
                return -2; // ������ͬ���û��޷��޸�
            }
            // ͬ���û�Ҳ���Ը�
            pwds[i].p_gid = gid;
            return 0;
        }
    }
    return -1;
}

// ��ʾ��ǰ�û��򿪵��ļ���Ϣ
void show_user_opened_files(){
    auto items = user_openfiles.find(cur_user)->second->items;
    std::cout << "filename" << "         fd" << "    count"<< "    offset" << std::endl;
    for(int i = 0; i < NOFILE; i++){
        if(items[i].f_count != 0)
            std::cout << system_openfiles[items[i].index_to_sysopen].fcb.d_name
                      << " " << items[i].index_to_sysopen
                      << "    " << items[i].f_count
                      << "    " << items[i].f_offset
                      << std::endl;
    }
}
// ��ʾ�����û��򿪵��ļ���Ϣ
void show_opened_files(){

    std::cout <<"uid" << "    filename" << "         fd" << "    count" << std::endl;
    for(auto user_openfile: user_openfiles){
        if(user_openfile.second == nullptr)
            continue;
        auto items = user_openfile.second->items;
        for(int i = 0; i < NOFILE; i++){
            if(items[i].f_count != 0)
                std::cout << user_openfile.second->p_uid
                          << "    " << system_openfiles[items[i].index_to_sysopen].fcb.d_name
                          << " " << items[i].index_to_sysopen
                          << "    " << items[i].f_count
                          << std::endl;
        }
    }
}
// ��ʾϵͳ�򿪱�
void show_sys_opened_files(){
    std::cout << "filename" << "         d_index" << "    count" << std::endl;
    for(int i = 0; i < SYSOPENFILE; i++){
        if(system_openfiles[i].i_count != 0){
            std::cout << system_openfiles[i].fcb.d_name
                      << " " << system_openfiles[i].fcb.d_index
                      << "    " << system_openfiles[i].i_count
                      << std::endl;
        }
    }
}