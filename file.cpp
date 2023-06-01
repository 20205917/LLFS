//
// Created by 86136 on 2023/6/1.
//

#include "RunningSystem.h"

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



inode *find_file(string addr) {
    dir *temp_dir = (dir *) cur_dir_inode->content;   //����Ŀ¼���
    unsigned int index; //��������ļ����ҵ���FCB��d_index
    int isInDir = 0;    //�ж��Ƿ���Ŀ¼��
    hinode final;//���Ա����˳�ѭ�������һ�����ļ���Ŀ¼���ڴ�i���
    if (addr[0] == '/') {//����·��
        addr = addr.substr(1, addr.length());
        temp_dir = (dir *) (iget(1)->content);
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

// Ӳ���Ӵ�����ʼ��Ϊ1
// ��Ҫ�����ļ�ƫ�������˴�δʵ��
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
    for (auto & system_openfile : system_openfiles) {//��ϵͳ�򿪱�ı���
        if (system_openfile.fcb.d_index == file_index && system_openfile.i_count != 0)
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
    //�ж��ļ��Ƿ��û���

    // ��ȡ�û��Ĵ򿪱�
    user_open_table *userOpenTable = user_openfiles[cur_user];

    // ʹ��fd��ȡ���ļ�
    struct user_open_item opened_file = userOpenTable->items[fd];
    // Ϊ0˵����ȡ����
    if(opened_file.f_count == 0)
        return -1; // �ļ���ʶ������

    hinode file_inode = opened_file.f_inode;
    file_inode->ifChange = 1;
    unsigned long offset = opened_file.f_offset;


    // д�ļ�
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

// ���û��Դ򿪵��ļ��ж�ȡ����
// ���ַ���ʽ��������
// û��ʵ��Ȩ���ж�
string readFile(int fd,int len) {
    //�ж��ļ��Ƿ��û���
    // ��ȡ�û��Ĵ򿪱�
    user_open_table *userOpenTable = user_openfiles[cur_user];
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
    userOpenTable->items[fd].f_offset +=len;

    return result;
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
