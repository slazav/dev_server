#ifndef DEVICE_H
#define DEVICE_H

#include <set>
#include <string>
#include <memory>

#include "err/err.h"
#include "opt/opt.h"
#include "drivers.h"
#include <mutex>

/*************************************************/

// A device object represents a device in the configuration
// file.

class Device {

  // Device driver
  std::shared_ptr<Driver> drv;

  // Connections which use the device
  std::set<uint64_t> users;

  // Device name
  std::string dev_name;

  // Driver name and args (for printing device information)
  std::string drv_name;
  Opt drv_args;

  // Mutex for locking device data
  std::mutex data_mutex;

  // Get lock for the mutex
  std::unique_lock<std::mutex> get_lock() {
    return std::unique_lock<std::mutex>(data_mutex);}

public:
  // Constructor
  Device( const std::string & dev_name,
          const std::string & drv_name,
          const Opt & drv_args);

  // Copy constructor
  Device(const Device & d);

  // Start using the device by a connection.
  // Open it if nobody else use it.
  void use(const uint64_t conn);

  // Stop using the device by a connection.
  // Close it if nobody else use it.
  void release(const uint64_t conn);

  // Do an action with the device
  std::string do_action(const std::string & act, const std::string & arg){
    return drv->do_action(act, arg); }

  // Print device information: name, users, driver, driver arguments.
  std::string print(const uint64_t conn=0) const;

};

#endif
