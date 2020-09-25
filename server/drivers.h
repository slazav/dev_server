#ifndef DRIVERS_H
#define DRIVERS_H

#include <string>
#include <map>
#include <memory>
#include <cstring>

#include "log/log.h"
#include "opt/opt.h"
#include "iofilter/iofilter.h"

/*************************************************/
// base class

class Driver {
protected:
  Opt opts;
  Driver(const Opt & opts): opts(opts) {}

public:
  // create and return a device driver (static function)
  static std::shared_ptr<Driver> create(
    const std::string & name, const Opt & args);

  virtual void open() = 0;
  virtual void close() = 0;
  virtual std::string cmd(const std::string & cmd,
                          const std::string & arg) = 0;

};

/*************************************************/
// test driver

struct Driver_test: Driver {
private:
  bool opened;
public:
  Driver_test(const Opt & opts): Driver(opts) {}
  void open()  override {opened=true;}
  void close() override {opened=false;}
  std::string cmd(const std::string & cmd,
                  const std::string & arg) override {
    if (!opened) throw Err() << "Test: sending command to a closed device";
    if (cmd != "ask") throw Err() << "unknown command: " << cmd;
    return arg;
  };
};

/*************************************************/
// spp driver

struct Driver_spp: Driver {
  std::shared_ptr<IOFilter> flt;
  std::string prog;
  char ch; // protocol special character
  int ver; // protocol version
  double open_timeout, read_timeout;

  Driver_spp(const Opt & opts): Driver(opts) {
    prog = opts.get("prog");
    open_timeout = opts.get<double>("open_timeout", 20.0);
    read_timeout = opts.get<double>("read_timeout", 5.0);
    if (prog == "") throw Err() << "Parameter -prog is empty or missing";
  }

  // read SPP message until #OK or #Error line
  std::string read_spp(double timeout = -1){
    if (!flt) throw Err() << "SPP: read from closed device";
    std::string ret;
    while (1){
      std::string l;

      // Read from SPP program with timeout.
      // Err is thrown if error happens.
      // Return -1 on EOF.
      int res = flt->getline(l, timeout);
      // line starts with the special character
      if (l.size()>0 && l[0] == ch){
        if (l.substr(1,7) == "Error: ") throw Err() << l.substr(8);
        if (l.substr(1,7) == "Fatal: ") throw Err() << l.substr(8);
        if (l.substr(1) == "OK") return ret;

        if (ret.size()>0) ret += '\n';
        if (l.size()>1 && l[1] == ch) ret += l.substr(1);
        else throw Err() << "SPP: symbol " << ch <<
          " in the beginning of a line is not protected: " << prog;
      }
      else {
        if (ret.size()>0) ret += '\n';
        ret += l;
      }
      if (res<0) break;
    }
    throw Err() << "SPP: no #OK or #Error message: " << prog;
  }

  // start SPP program
  void open() override {
    flt.reset(new IOFilter(prog));
    try {

      // first line: <symbol>SPP<version>
      std::string l;
      flt->getline(l, open_timeout);
      if (l.size()<5 || l[1]!='S' || l[2]!='P' || l[3]!='P')
        throw Err() << "SPP: unknown protocol, header expected: " << prog;
      ch  = l[0];
      int ver = str_to_type<int>(l.substr(4));
      if (ver!=1 && ver!=2) throw Err() << "SPP: unsupported protocol version: " << prog;
      read_spp(open_timeout); // ignore message, throw errors
    }
    catch (Err e) {
      close();
      throw e;
    }
  }

  // close SPP program
  void close() override {
    if (!flt) return;
    flt->close_input();
    flt->kill();
    flt.reset();
  }

  // send a command to the device and read answer
  std::string cmd(const std::string & cmd,
                  const std::string & arg) override {
    if (!flt) throw Err() << "SPP: writing to a closed device";
    if (cmd != "ask") throw Err() << "unknown command: " << cmd;
    flt->ostream() << arg << "\n";
    flt->ostream().flush();
    return read_spp(read_timeout);
  }
};

#endif
