#ifndef DRIVERS_H
#define DRIVERS_H

#include <string>
#include <map>
#include <memory>
#include <cstring>

#include "opt/opt.h"

/*************************************************/
// base class

class Driver {
public:

  // create and return a device driver (static function)
  static std::shared_ptr<Driver> create(
    const std::string & name, const Opt & args);

  virtual void open()  = 0;
  virtual void close() = 0;
  virtual std::string cmd(const std::string & cmd) = 0;

};

/*************************************************/
// test driver

struct Driver_test: Driver {
  void open() override {}
  void close() override {}
  std::string cmd(const std::string & cmd) override {return cmd;};
};

#endif
