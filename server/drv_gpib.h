#ifndef DRV_GPIB_H
#define DRV_GPIB_H

#include <gpib/ib.h>
#include "drv.h"
#include "opt/opt.h"

/*************************************************/
/*
 * Access to a GPIB device using linux-gpib library.
 *

Options:

  -addr <N>      -- GPIB address.
                    Required.

  -board <N>     -- GPIB board number.
                    Default: 0.

  -timeout <str> -- I/O timeout: none, 10us, 30us, 100us ... 300s, 1000s
                    Default 3s.

  -open_timeout <str> -- Open timeout, same values as for -read_timeout.
                         Default 3s.

  -eot (1|0)     -- Enable/disable assertion of EOI line with last data byte

  -eos_mode <v>  -- Set end-of-string mode (none, read, write, bin)
                    Only works with -eos option. Default: none.

  -eos (0..255)  -- Set end-of-string character.
                    Default: do not set

  -secondary (1|0) -- Set secondary GPIB address.

  -bufsize <N>   -- Buffer size for reading. Maximum length of read data.
                    Default: 4096

  -errpref <str> -- Prefix for error messages.
                    Default: "gpib: "

  -idn <str>     -- Override output of *idn? command.
                    Default: empty string, do not override.

  -add_ch <N>    -- Add character to each message sent to the device.
                    Default: '\n'

  -trim_ch <N>   -- Remove character from the end of recieved messages.
                    Default: '\n'

*/

class Driver_gpib: public Driver {
protected:
  int dh; // GPIB device handler
  size_t bufsize;
  std::string errpref,idn;
  int add,trim;

  // convert timeout
  int get_timeout(const std::string & s);

public:

  Driver_gpib(const Opt & opts);
  ~Driver_gpib();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
