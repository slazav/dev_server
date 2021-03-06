#ifndef DRV_NET_H
#define DRV_NET_H

#include <memory>
#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

/*************************************************/
/*
 * Driver `net` -- network devices
 *

Defaults correspond to LXI raw protocol.

Driver reads answer from the device only if there is a question mark '?'
in the message.

Parameters:

* `-addr`          -- Network address or IP.
                      Required.
* `-port <N>`      -- Port number.
                      Default: "5025" (lxi raw protocol).
* `-timeout <N>`   -- Read timeout, seconds. No timeout if <=0.
                      Default 5.0.
* `-bufsize <N>`   -- Buffer size for reading. Maximum length of read data.
                      Default: 4096
* `-errpref <str>` -- Prefix for error messages.
                      Default: "IOSerial: "
* `-idn <str>`     -- Override output of *idn? command.
                      Default: empty string, do not override.
* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (if there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: qmark1w.
* `-add_str <v>`   -- Add string to each message sent to the device.
                      Default: "\n"
* `-trim_str <v>`  -- Remove string from the end of received messages.
                      Default: "\n"
*/

class Driver_net: public Driver {
protected:
  int sockfd; // file descriptor for the network socket
  size_t bufsize;
  double timeout;
  std::string errpref,idn;
  std::string add,trim;
  read_cond_t read_cond;

public:

  Driver_net(const Opt & opts);
  ~Driver_net();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
