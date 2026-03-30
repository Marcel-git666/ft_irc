#!/bin/bash

SERVER="127.0.0.1"
PORT="6667"
PASSWORD="123"
CHANNEL="#stress"
CLIENTS=10

for i in $(seq 1 $CLIENTS); do
(
    {
	    sleep 0.5
	    printf "PASS %s\r\n" "$PASSWORD"
	    printf "NICK user$i\r\n"
	    printf "USER user$i 0 * :User $i\r\n"
	    sleep 1
	    printf "JOIN #stress\r\n"
	    sleep 2
	    printf "PRIVMSG #stress :Hello from user$i\r\n"
        sleep 3
        printf "QUIT :bye from user$i\r\n"
    } | nc $SERVER $PORT > /dev/null 2>&1
) &
done

wait
echo "✅ Finished testing $CLIENTS clients."