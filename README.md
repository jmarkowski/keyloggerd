# keyloggerd

`keyloggerd` is a background key logging daemon that runs for linux.


# Installation

Currently this is not configured within the autotools framework, which means the
installation process is old school:

    $ git clone https://github.com/jmarkowski/keyloggerd.git
    $ cd keyloggerd
    $ make

This will compile the `keyloggerd` program.

Note: this will not *install* the program, I have yet to do this ;) However,
if you manually want to install it, you may simply copy the `keyloggerd` program
into /usr/local/bin (or equivalent).


# Quickstart

    $ sudo ./keyloggerd

`keyloggerd` must be run as a super user because it requires detection of the
input keyboard events from `/dev/input/event0` (default)


# Where are the logs?

`keyloggerd` generates two files:

1. `/var/run/keyloggerd.pid`, which contains the PID of the daemon
2. `/tmp/key.log`, which contains the keys that have been pressed

If you find that nothing is being logged to `key.log`, no matter how many keys
you bash, you may need to change your input device. For example:

    $ sudo ./keyloggerd --keyboard-device /dev/input/event1

A good hint on how to discover your input device is to look in the
`/var/log/Xorg.0.log` file.

    $ cat /var/log/Xorg.0.log | grep keyboard

The `key.log` will be overwritten every time the `keyloggerd` daemon runs. If
instead you'd like to append to it each time, you may use the `--append` switch.

    $ sudo ./keyloggerd --append

## Daemon Status

The keylogger daemon prints status information into the system log, which may be
viewed as follows

    $ journalctl -xe

This is useful for debugging any issues you may be experiencing.

## Secret Key Sequences

Because the keys are logged, you may use the following sequence of keys to
control the daemon:

Action                      | Key Sequence
----------------------------|-----------------------------------
Kill `keyloggerd` daemon    | ESC, ESC, ESC
Start / stop key logging    | RIGHTSHIFT, RIGHTSHIFT, RIGHTSHIFT

These secret key sequences are configurable via `keyloggerd.conf`.

## Configuration

A `keyloggerd.conf` file may be used to configure the defaults for the
keyloggerd daemon on startup.

A default conf file is included in the repository.


# Interpreting the Key Log

All keys are logged in `/tmp/key.log` by default unless otherwise specified via
the configuration file or the `-f` option.

You will notice that "invisible" keys are recorded in surrounding "<" and ">"
tags. For example, when the escape key is logged, it is recorded as "\<ESC>".

Any unknown keys will be recorded as a number surrounded by the "<" and ">"
tags.

By default, any backspaces will be recorded using the ASCII backspace character,
and thus work as if the previous character was being "deleted". If you want to
explicitly log the backspace as a character, you may specify it via
`--backspace-char`, or via the `backspace_char` field in the configuration file.


# Wish list

This project is a work in progress, and so there are still several things I'd
like to do:

*  Setup with autotools
*  Use signals to re-read conf while running
*  Allow keys to be configured via `keyloggerd.conf`
