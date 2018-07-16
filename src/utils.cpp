#include <systemc.h>
#include <utils.h>
#include<vector>

#include <task.h>


void printl(const char* file_name, const char* module_name, const char* fmt...){
  cout << "@" << sc_time_stamp() << ": " << file_name << ":" << module_name <<  "  ";
  va_list args;
  va_start(args, fmt);

  while (*fmt != '\0') {
      if (*fmt == '%' && *(fmt+1) == 'd') {
          int i = va_arg(args, int);
          std::cout << i;
          fmt++;
      } else if (*fmt == '%' && *(fmt+1) == 'c') {
          // note automatic conversion to integral type
          int c = va_arg(args, int);
          std::cout << static_cast<char>(c);
          fmt++;
      } else if (*fmt == '%' && *(fmt+1) == 'f') {
          double d = va_arg(args, double);
          std::cout << d;
          fmt++;
      } else if (*fmt == '%' && *(fmt+1) == 's') {
        char *s = va_arg(args, char *);
        std::cout << s;
        fmt++;
      }
      else {
        std::cout << *fmt;
      }
      ++fmt;
  }
  std::cout << std::endl;
  va_end(args);
}

void read_tasks(std::string filename, std::vector<task> * tasks) {
  std::ifstream fin(filename.c_str(), std::ifstream::in);
  task t;

  while (fin>>t)
    tasks->push_back(t);
}
