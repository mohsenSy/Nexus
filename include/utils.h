#ifndef _UTILS_H
#define _UTILS_H

#include <iostream>
#include <cstdarg>
#include<vector>

#include <task.h>

#ifdef DEBUG
#define PRINTL(fmt, ...) printl(__FILE__, name(), fmt, __VA_ARGS__)
#else
#define PRINTL(fmt, ...) {}
#endif

void printl(const char* file_name, const char* module_name, const char* fmt...);

void read_tasks(std::string filename, std::vector<task> * tasks);

#endif
