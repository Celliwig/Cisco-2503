#!/bin/bash -x

sudo socat PTY,link=/dev/cisco_emu_dce,user=charles,group=charles PTY,link=/dev/cisco_emu_dte,user=charles,group=charles
