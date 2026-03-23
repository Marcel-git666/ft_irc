#!/bin/bash

SERVER="127.0.0.1"
PORT="6667"
PASSWORD="123"
CHANNEL="#stress"
CLIENTS=3

for i in $(seq 1 $CLIENTS); do
(
    {
	    sleep 0.1
	    printf "PASS %s\r\n" "$PASSWORD"
	    printf "NICK user$i\r\n"
	    printf "USER user$i 0 * :User 1\r\n"
	    sleep 0.2
	    printf "JOIN #stress\r\n"
	    sleep 1
	    printf "PRIVMSG #stress :Hello from user1\r\n"
        sleep 2
        printf "QUIT :bye\r\n"
    } | nc $SERVER $PORT > /dev/null 2>&1
) &
done

wait
echo "✅ Finished testing $CLIENTS clients."