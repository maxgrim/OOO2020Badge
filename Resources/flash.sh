#!/usr/bin/env bash
esptool.py --chip esp8266 --port "$1" --baud 115200 write_flash 0 $2 1048576 $3
