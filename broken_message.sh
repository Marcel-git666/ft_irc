#!/bin/bash

SERVER="127.0.0.1"
PORT="6667"
PASSWORD="123 "
CHANNEL="#stress"

(
	sleep 0.1
    echo "PASS $PASSWORD"
    echo "NICK user1 "
    echo "USER user1 0 * :User 1 "
    sleep 0.2
  printf "JO"
  sleep 0.2
  printf "IN #stre"
  sleep 0.2
  printf "ss\r\n"
) | nc 127.0.0.1 6667