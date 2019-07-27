#ifndef _UTILS_H
#define _UTILS_H

#include <iostream>
#include <cstdarg>
#include<vector>

#include <task.h>

#ifdef DEBUG
#define PRINTL(fmt, ...) printl(__FILE__,__LINE__, name(), fmt, __VA_ARGS__)
#else
#define PRINTL(fmt, ...) {}
#endif

#define LOG(file_name, fmt, ...) printll(__FILE__, name(), file_name, fmt, __VA_ARGS__);

void printl(const char* file_name, const char* module_name, const char* fmt...);
void printl(const char* file_name,const int line, const char* module_name, const char* fmt...);
void printll(const char* file_name, const char* module_name, std::string name, const char* fmt...);

void read_tasks(std::string filename, std::vector<task> * tasks);

#endif
