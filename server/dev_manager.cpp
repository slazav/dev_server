#include <iostream>
#include <fstream>
#include <unistd.h>

#include "err/err.h"
#include "log/log.h"
#include "read_words/read_words.h"
#include "locks.h"
#include "drivers.h"
#include "dev_manager.h"

#define SRVDEV "SERVER"

/*************************************************/
Device::Device( const std::string & dev_name,
        const std::string & drv_name,
        const Opt & drv_args):
  std::shared_ptr<Driver>(Driver::create(drv_name, drv_args)),
  Lock("manager:" + dev_name),
  dev(dev_name) {
}

void
Device::open(const uint64_t conn){
  if (users.count(conn)>0) return; // device is opened and used by this connection
  lock();
  bool do_open = users.empty();          // device need to be opened
  users.insert(conn);
  if (do_open) {
    (*this)->open();
    Log(2) << "#" << conn << "/" << dev << ": open device";
  }
  unlock();
}

void
Device::close(const uint64_t conn){
  if (users.count(conn)==0) return; // device is not used by this connection
  lock();
  users.erase(conn);
  bool do_close = users.empty();
  if (do_close){
    (*this)->close();
    Log(2) << "#" << conn << "/" << dev << ": close device";
  }
  unlock();
}

std::string
Device::cmd(const std::string & cmd, const std::string & arg){
  if (cmd == "ask"){
    lock();
    auto ret = (*this)->cmd(arg);
    unlock();
    return ret;
  }
  throw Err() << "unknown command: " << cmd;
}

/*************************************************/
DevManager::DevManager(): Lock("manager"){ }

/*************************************************/
std::vector<std::string>
DevManager::parse_url(const std::string & url){
  std::vector<std::string> ret(3);
  size_t p1(0), i(0);
  if (url.size()>0 && url[0]=='/') p1++; // skip leading /
  for (i=0; i<2; ++i){
    size_t p2 = url.find('/', p1);
    if (p2 == std::string::npos) break;
    ret[i] = url.substr(p1, p2-p1);
    p1=p2+1;
  }
  ret[i] = url.substr(p1);
  return ret;
}

/*************************************************/
void
DevManager::conn_open(const uint64_t conn){
  Log(2) << "#" << conn << ": open connection";
}

void
DevManager::conn_close(const uint64_t conn){
  // go through all devices, close ones which are not needed
  for (auto & d:devices) d.second.close(conn);
  Log(2) << "#" << conn << ": close connection";
}

/*************************************************/
std::string
DevManager::run(const std::string & url, const uint64_t conn){
  auto vs = parse_url(url);
  std::string dev = vs[0];
  std::string cmd = vs[1];
  std::string arg = vs[2];

  Log(3) << "#" << conn << "/" << dev << " >> " << cmd << ": " << arg;

  try { // throw errors with code=1 for normal return

    if (dev == "") throw Err() << "empty device";

    // special device SERVER
    if (dev == SRVDEV){
      if (cmd == "get_log_level") {
         throw Err(1) << type_to_str(Log::get_log_level());
      }
      if (cmd == "set_log_level"){
         lock();
         Log::set_log_level(str_to_type<int>(arg));
         unlock();
         throw Err(1) << type_to_str(Log::get_log_level());
      }
      if (cmd == "open_dev"){
        if (devices.count(arg) == 0)
          throw Err() << "unknown device: " << dev;
        Device & d = devices.find(arg)->second;
        d.open(conn);
        throw Err(1);
      }
      if (cmd == "usleep"){
        int t = str_to_type<int>(arg);
        usleep(t);
        throw Err(1) << t;
      }
      if (cmd == "repeat") {
        throw Err(1) << arg;
      }
      throw Err() << "unknown server command: " << cmd;
    }

    // other devices
    if (devices.count(dev) == 0)
      throw Err() << "unknown device: " << dev;
    Device & d = devices.find(dev)->second;
    d.open(conn);
    throw Err(1) << d.cmd(cmd, arg);
  }
  catch (Err e){
    if (e.code() == 1){
      Log(3) << "#" << conn << "/" << dev << " << answer: " << e.str();
      return e.str();
    }
    Log(2) << "#" << conn << "/" << dev << " << error: " << e.str();
    throw e;
  }
}

/*************************************************/
void
DevManager::read_conf(const std::string & file){
  std::map<std::string, Device> ret;
  int line_num[2] = {0,0};
  std::ifstream ff(file);
  if (!ff.good()) throw Err()
    << "can't open configuration: " << file;

  Log(1) << "Reading configuration file: " << file;

  try {
    while (1){
      std::vector<std::string> vs = read_words(ff, line_num, false);
      if (vs.size() < 1) break;
      if (vs.size() < 2) throw Err()
        << "expected: <device name> <driver name> [-<parameter> <value>]";
      std::string dev = vs[0];
      std::string drv = vs[1];
      std::vector<std::string> args(vs.begin()+2, vs.end());
      if ((vs.size()-2)%2 != 0) throw Err()
        << "even-size list of [-<parameter> <value>] pairs expected";
      Opt opt;
      for (int i=2; i+1<vs.size(); i+=2){
        if (vs[i].size()<2 || vs[i][0]!='-') throw Err()
          << "parameter name should be prefixed with \"-\" "
          << "and contain at least one character: " << vs[i];
        opt.put(vs[i], vs[i+1]);
      }

      // do not allow empty devices
      if (dev == "") throw Err() << "empty device";

      // do not allow SERVER device
      if (dev == SRVDEV) throw Err() << "can't redefine " << SRVDEV << " device";

      // does this device exists
      if (ret.count(dev)>0) throw Err()
        << "duplicated device name: " << dev;

      // add device information
      ret.emplace(dev, Device(dev,drv,opt));

    }
  } catch (Err e){
    throw Err() << "bad configuration file "
                << file << " at line " << line_num[0] << ": " << e.str();
  }

  Log(1) << ret.size() << " devices configured";

  lock();
  devices = ret; // apply the configuration only if no errors have found.
  unlock();
}

