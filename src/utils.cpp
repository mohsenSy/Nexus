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

void printl(const char* file_name,const int line, const char* module_name, const char* fmt...){
  cout << "@" << sc_time_stamp() << ": " << file_name << ":" << module_name << ":" << line <<  "  ";
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

void printll(const char* file_name, const char* module_name,std::string name, const char* fmt...){
  ofstream ofile;
  ofile.open(name, ios::app);
  ofile << "@" << sc_time_stamp() << ": " << file_name << ":" << module_name <<  "  ";
  va_list args;
  va_start(args, fmt);

  while (*fmt != '\0') {
      if (*fmt == '%' && *(fmt+1) == 'd') {
          int i = va_arg(args, int);
          ofile << i;
          fmt++;
      } else if (*fmt == '%' && *(fmt+1) == 'c') {
          // note automatic conversion to integral type
          int c = va_arg(args, int);
          ofile << static_cast<char>(c);
          fmt++;
      } else if (*fmt == '%' && *(fmt+1) == 'f') {
          double d = va_arg(args, double);
          ofile << d;
          fmt++;
      } else if (*fmt == '%' && *(fmt+1) == 's') {
        char *s = va_arg(args, char *);
        ofile << s;
        fmt++;
      }
      else {
        ofile << *fmt;
      }
      ++fmt;
  }
  ofile << std::endl;
  va_end(args);
  ofile.close();
}

void read_tasks(std::string filename, std::vector<task> * tasks) {
  std::ifstream fin(filename.c_str(), std::ifstream::in);
  task *t = new task();
  while (fin>>*t){
    int addr;
    char c;
    for (int i = 0; i < t->input_args; i++) {
      fin >> c >> addr;
      t->set_input_arg(i, (mem_addr)addr);
    }
    for (int i = 0; i < t->output_args; i++) {
      fin >> c >> addr;
      t->set_output_arg(i, (mem_addr)addr);
    }
    tasks->push_back(*t);
    //std::cout << "Read task with id " << t->id << std::endl;
    t = new task();
  }
}
