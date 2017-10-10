# keyloggerd

`keyloggerd` program is a background key logging daemon that runs for linux.

# Installation

Currently this is not configured within the autotools framework, which means the
installation process is old school:

```
$ git clone https://github.com/jmarkowski/keyloggerd.git
$ cd keyloggerd
$ make
```

This will compile the `keyloggerd` daemon.

# Usage

`keyloggerd` must be run as a super user because it requires detection of the
input keyboard events from `/dev/input/event0` (default)

```
$ sudo ./keyloggerd
```

This results in two files being created:

1. `/var/run/keyloggerd.pid` containing the PID of the daemon
2. `/tmp/key.log` that is now logging the keys pressed

If you find that nothing is getting logged to `key.log`, no matter how many keys
you bash, you may need to change your input device. For example:

```
$ sudo ./keyloggerd --keyboard-device /dev/input/event1
```

A good hint on how to discover your input device is to look in the
`/var/log/Xorg.0.log` file.

```
$ cat /var/log/Xorg.0.log | grep keyboard
```

The `key.log` will be overwritten every time the `keyloggerd` daemon runs. If
instead you'd like to append to it each time, you may use the `--append` switch.

```
$ sudo ./keyloggerd --append
```

## Daemon Logs

The keylogger daemon prints log information into the system log, which may be
viewed as follows

```
$ journalctl -xe
```

## Secret Key Sequences

Because the keys are logged, you may use the following sequence of keys to
control the daemon:

Key Sequence | Action
-------------|-------
ESC, ESC, ESC | Kill `keyloggerd` daemon
RSHIFT, RSHIFT, RSHIFT | Start / stop key logging


# Wish list

This project is a work in progress, and so there are still several things I'd
like to do:

* Add timestamps
* Add support for a conf file to configure key sequences
* Add ability to tell if capslock is enabled or not
* Setup with autotools
