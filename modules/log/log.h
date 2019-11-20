#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <iostream>
#include "err/err.h"

/*************************************************/
// Log class

class Log {
  static std::ostream *log;  // log stream
  static std::ofstream flog; // log stream for file logs
  static int log_level;
  bool empty;

public:

  static void set_log_file(const std::string & fname);

  static void set_log_level(const int lvl) { log_level = lvl; }

  static int get_log_level(){ return log_level; }

  Log(int l): empty(l>log_level) { }

  ~Log() { if (!empty) (*log) << "\n";}

  /// Operator << for error messages.
  template <typename T>
  Log & operator<<(const T & o){
    if (!empty) (*log) << o;
    return *this;
  }

};

#endif
