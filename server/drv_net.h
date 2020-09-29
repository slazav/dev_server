#ifndef DRV_NET_H
#define DRV_NET_H

#include <memory>
#include "drv.h"
#include "opt/opt.h"

/*************************************************/
/*
 * Access to a network device.
 * Defaults correspond to LXI raw protocol.
 *
 * see https://beej.us/guide/bgnet/html//index.html

Options:
  -addr    -- Network address or IP.
              Required.
  -port    -- Port number.
              Default: "5025" (lxi raw protocol).
  -timeout -- Read timeout, seconds. No timeout if <=0.
              Default 5.0.
  -bufsize -- Buffer size for reading. Maximum length of read data.
              Default: 4096
  -errpref -- Prefix for error messages.
              Default: "IOSerial: "
  -add_ch <N>  -- Add character to each message sent to the device.
                  Default: '\n'
  -trim_ch <N> -- Remove character from the end of recieved messages.
                  Default: '\n'
*/

class Driver_net: public Driver {
  int sockfd; // file descriptor for the network socket
  size_t bufsize;
  double timeout;
  int add,trim;
  std::string errpref;

public:

  Driver_net(const Opt & opts);
  ~Driver_net();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
