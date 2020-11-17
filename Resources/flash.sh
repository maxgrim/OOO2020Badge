#!/usr/bin/env bash
# Usage: ./flash.sh <port> <firmware> <spiffs>
esptool.py erase_flash

echo -e "\r\nFlash erased, now put the badge in flash mode again"
read -p "Press enter to continue..."

esptool.py --chip esp8266 --port "$1" --baud 115200 write_flash 0 $2 1048576 $3
