## Device server
---

This is a server for accessing devices and programs in sophisticated
experimental setups. Previous version was a tcl library (
https://github.com/slazav/tcl-device ), but that approach had a few
limitations.

All operations are done using "devices". They can be physical devices
connected via some interface, or programs (which can use other
devices if needed).

The server has a configuration file `/etc/device_server.txt` which lists
all avalible devices. For example, line `generator gpib -board 0 -address
6` says that communication with device `generator` is done using driver
gpib with parameters `-board 0 -address 6`.

### HTTP server

Users communicate with the server using GET requests of HTTP protocol.
URLs with three-components are used: `<device>`, `<command>`,
`<argument>`. For example, a request to
`http://<server>:<port>/generator/ask/FREQ?`" sends phrase `FREQ?` to
the device `generator` and returns answer. Commands are driver-specific.

On success a response with code 200 and answer of the device in the
message body is returned. On error a response with code 400 is returned.
Error description is written in `Error` header and in the mesage body.

The server does not know what it sends to a device and what answer is
expected, it just provides connection. For the next layer see DeviceRole
library. It defines sertain "roles" for devices with common high-level
commands (e.g. "generator" role has a "set frequency" command).

### Locking

Server and device drivers provide IO locking (one device can be used by a
severel programs or threads without collisions) and optional high-level
locking (one program can grab a device for a long time).

Server provides logging of all device communications: if there is a file
`/var/log/device_server/<name>` then all communication with the device
<name> is appended to this file. This allows to start/stop logging
without restarting and modifing programs.

## TODO

- device and resource locks
- lock timeouts
- logging
- session-based device management
- grabbing devices
- authorisation

### Configuration file

Configuration file contains one line per device. Empty lines and
comments (starting with `#`) are allowed. A few lines can be joined by
adding symbol `\` before end of line. Words can be quoted by single
or double quotes. Special symbols (`#`, `\`, quotes) can be typed by
adding `\` in front of them. Case of characters is important.

Each non-empty line should have the form:
```
<device name> <driver name> [<paramter> ...]
```

Parameters are pairs of words, first word in each pair should have prefix
`-`. Example: `-key1 val1 -key2 -val2`

Parameters are driver-specific.

### Special device SERVER

Device with name SERVER is a special device for controlling the device server
itself. It can not be redifined in the configuration file.

Supported commands:

* `get_log_level` -- Get logging level of the server (0 - no messages, 1 -
server messages,  2 - connections, 3 - communications with devices).

* `set_log_level` -- Set logging lavel. Argument is integer.

* `open_dev` -- Open a device. Argument is device name. Usually a device
is opened on demand, then a command is sent to it. This command can be
used to open the device before sending any command to it.

* `close_dev` -- Close a device (if no other sessions use it). Argument is
device name. Usually devices are closed when session is ended and no
other sessions are using the device. This command can be used to close the
device without closing the session. If some other session uses the device it
will stay opened.

* `usleep` -- Sleep for some time. Number of microseconds is taken from the argument
and returned in the answer.

* `repeat` -- Repeat the argument in the answer.


### Driver `test` -- a dummy driver for tests

Supported configuration options: none

Supported commands:
* ask -- just repeat the message

