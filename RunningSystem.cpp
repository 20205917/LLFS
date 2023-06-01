//
// Created by 86136 on 2023/5/25.
//

#include <string>
#include <iomanip>
#include <algorithm>
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
        memset(pwds[i].password, 0, 12);
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
    for (auto & hinode : hinodes) {
        while (hinode->i_forw) {
            iput(hinode->i_forw);
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
        memset(pwds[j].password, 0, 12);
    }
    fseek(disk, 0, SEEK_SET);
    fwrite(pwds, sizeof(PWD), PWDNUM, disk);
}

// �ػ�������
void halt() {
    // ��鵱ǰ�û����ͷ�
    for (const auto &data: user_openfiles) {
        user_open_table *tmp = data.second;
        for (auto & item : tmp->items) {
            if (item.f_inode == nullptr)
                continue;
            int id = item.index_to_sysopen;
            system_openfiles[id].i_count--;
            if (system_openfiles[id].i_count == 0)
                system_openfiles[id].fcb.d_index = 0;
        }
    }
    user_openfiles.clear();

    // ��ջ���
    for (auto & hinode : hinodes) {
        while (hinode->i_forw) {
            iput(hinode->i_forw);
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



//��ʾ��ǰ�û�ss
string whoami() {
    return cur_user;
}


// չʾ�����û�
void show_all_users(){
    std::cout << "uid" << "     gid" << "     pwd" << std::endl;
    for(int i = 0; i < USERNUM; i++){
        if(pwds[i].password[0] != '\0'){
            std::cout << pwds[i].p_uid << "       "
            << pwds[i].p_gid << "       "
            << pwds[i].password << std::endl;
        }
    }
}

// չʾ���е�¼�û�
void show_login_users(){
    std::cout << "uid" << "     gid" << "     pwd" << std::endl;
    for(const auto& user: user_openfiles){
        for(int i = 0; i < USERNUM; i++){
            if(!strcmp(pwds[i].password, user.first.c_str())){
                std::cout << pwds[i].p_uid << "       "
                          << pwds[i].p_gid << "       "
                          << pwds[i].password << std::endl;
            }
        }
    }
}



// ��ʾ��ǰ�û��򿪵��ļ���Ϣ
void show_user_opened_files(){
    auto items = user_openfiles.find(cur_user)->second->items;
    std::cout << "filename" << "          fd" << "   count"<< "    offset" << std::endl;
    for(int i = 0; i < NOFILE; i++){
        if(items[i].f_count != 0)
            std::cout << setiosflags(ios::left)  << setw(17) << system_openfiles[items[i].index_to_sysopen].fcb.d_name
                      << items[i].index_to_sysopen
                      << "     " << items[i].f_count
                      << "        " << items[i].f_offset
                      << std::endl;
    }
}
// ��ʾ�����û��򿪵��ļ���Ϣ
void show_opened_files(){

    std::cout <<"uid" << "    filename" << "         fd" << "    count" << std::endl;
    for(const auto& user_openfile: user_openfiles){
        if(user_openfile.second == nullptr)
            continue;
        auto items = user_openfile.second->items;
        for(int i = 0; i < NOFILE; i++){
            if(items[i].f_count != 0)
                std::cout << setiosflags(ios::left)  << user_openfile.second->p_uid
                          << "      "<< setw(17) << system_openfiles[items[i].index_to_sysopen].fcb.d_name
                          << " " << items[i].index_to_sysopen
                          << "    " << items[i].f_count
                          << std::endl;
        }
    }
}
// ��ʾϵͳ�򿪱�
void show_sys_opened_files(){
    std::cout << "filename" << "         d_index" << "    count" << std::endl;
    for(auto & system_openfile : system_openfiles){
        if(system_openfile.i_count != 0){
            std::cout << setiosflags(ios::left) << setw(17) << system_openfile.fcb.d_name
                      << system_openfile.fcb.d_index
                      << "          " << system_openfile.i_count
                      << std::endl;
        }
    }
}

//·���Ƿ�Ϸ�
//Ŀ¼�Ƿ�Ϸ�  0- �������� 1-Ŀ¼ 2-��Ŀ¼�ļ�
int judge_path(string pathname) {
    string Subpathname;
    int first;//��һ�γ���'/'��λ��
    if (pathname.empty() || (pathname[pathname.length() - 1] == '/'&&pathname.length()!=1))        //���һ���ַ�Ϊ'/'�Լ�stringΪ��ʱ��
        return 0;
    if (pathname[0] == '/')//����·��
        pathname = pathname.substr(1, pathname.length());      //��ȥ��һ���ַ�'/'
    //����˳��ַ����һ��Ŀ¼���ж��ļ����Ƿ�Ϸ��Լ��Ƿ����������'/'
    while (pathname.find_first_of('/') != string::npos) {
        if (pathname[0] == '/')        //˵��������������'/'
            return 0;
        first = pathname.find_first_of('/');
        Subpathname = pathname.substr(0, first);
        pathname = pathname.substr(first + 1, pathname.length());
        if (Subpathname.find('.') != string::npos)     //Ŀ¼���г���'.'
            return 0;
    }
    //�߳�����ѭ�����pathnameΪ���һ�����ļ���
    if (pathname.find('.') == string::npos)        //˵������'.'��ΪĿ¼��
        return 1;

    int count = std::count(pathname.begin(), pathname.end(), '.');
    if (count > 1)         //�ļ����в����Ժ���һ�����ϵ�'.'
        return 0;
    return 2;
}