//
// Created by 86136 on 2023/5/25.
//
#include "RunningSystem.h"
#include <vector>

bool Split(vector<char*>* token,char* order){
    if(order[0] == 0)
        return false;
    if(order[0]==' ')
        return false;
    token->push_back(std::strtok(order," "));
    while(token->back()!=nullptr){
        token->push_back(std::strtok(nullptr," "));
    }
    token->pop_back();
    return true;
}
int toUnicode(const char* str)
{
    return str[0] + (str[1] ? toUnicode(str + 1) : 0);
}
