#ifndef DRV_SERIAL_SIMPLE_H
#define DRV_SERIAL_SIMPLE_H

/*************************************************/
/*
 * Seraldriver with reasonable default settings.
 * Should wirk with old Agilent/HP devices.
 *

Options:

  Serial port setup.

  -dev <name>   -- Serial device filename (e.g. /dev/ttyUSB0)
                   Required.

  -timeout <v>  -- Read timeout, seconds [0 .. 25.5]
                   Default: 5.0

  -sfc (0|1)    -- Software flow control.
                   Default: 1

  -errpref      -- error prefix (default "ASM340: ")

  -idn <str>     -- override output of *idn? command
                    Default: do not override.

*/

#include "drv_serial.h"

class Driver_serial_simple: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "sfc", "errpref"});
    Opt o(opts);
    o.put("speed",  9600);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("vmin",   0); // should be set with timeout
    o.put("ndelay", 0); // should be set with timeout
    o.put("icrnl",  1); // convert CR->NL on input
    o.put("raw",     1); // raw mode!
    o.put("delay", 0.1); // 100ms delay after write
    o.put("opost",   0); // no output postprocessing
    o.put("add_str", "\n"); // add NL to each sent message
    o.put("trim_str","\n"); // trim NL from each recieved message
    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0); // default timeout
    o.put_missing("sfc", 1);       // default software flow control
    return o;
  }
public:
  Driver_serial_simple(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif