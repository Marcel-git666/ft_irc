#!/bin/bash

SERVER="127.0.0.1"
PORT="6667"
PASSWORD="123 "
CHANNEL="#stress"
CLIENTS=200

for i in $(seq 1 $CLIENTS); do
(
    {
        sleep 0.1
        echo "PASS $PASSWORD"
        echo "NICK user$i"
        echo "USER user$i 0 * :User $i"
        sleep 0.2
        echo "JOIN $CHANNEL"
        sleep 1
        echo "PRIVMSG $CHANNEL :Hello from user$i"
        sleep 2
        echo "QUIT :bye"
    } | nc $SERVER $PORT > /dev/null 2>&1
) &
done

wait
echo "✅ Finished testing $CLIENTS clients."