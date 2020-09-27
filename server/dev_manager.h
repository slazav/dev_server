#ifndef DEV_MANGR_H
#define DEV_MANGR_H

#include <microhttpd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <shared_mutex> // C++14

#include "err/err.h"
#include "log/log.h"
#include "opt/opt.h"
#include "device.h"

class DevManager {

  // All devices (from configuration file):
  std::map<std::string, Device> devices;

  // Mutex for locking data
  typedef std::shared_timed_mutex mutex_t;
  mutex_t data_mutex;

  // Get lock for the mutex (for writers)
  std::unique_lock<mutex_t> get_lock() {
    return std::unique_lock<mutex_t>(data_mutex);}

  // Get shared lock for the mutex (for readers)
  std::shared_lock<mutex_t> get_sh_lock() {
    return std::shared_lock<mutex_t>(data_mutex);}

public:

  // number of devices (for tests)
  size_t size() const {return devices.size();}

  // open connection callback:
  void conn_open(const uint64_t conn);

  // close connection callback:
  void conn_close(const uint64_t conn);

  // Process a request from HTTP server.
  // Arguments:
  // - act:  action (URL without arguments in GET request)
  // - opts: options (arguments from the url)
  // - conn: connection ID
  std::string run(const std::string & act, const Opt & opts, const uint64_t conn);

  // Read configuration file, update `devices` map.
  // Throw exception on errors.
  void read_conf(const std::string & file);

  // Split url (action/argument/message), return vector<string> with 3 elements
  static std::vector<std::string> parse_url(const std::string & url);

};

#endif
