#!/bin/bash

PROG="./receiver-tcp"
PORT=`shuf -i 10000-65000 -n 1`
PORT_C=`shuf -i 10000-65000 -n 1`

echo -n "test 01 - program without arg: "
$PROG > /dev/null 2>&1 && echo "KO -> exit status $? instead of 1" && exit 1 
echo ".................OK"

echo -n "test 02 - invalid port number: "
$PROG 65001 > /dev/null 2>&1 && echo "KO -> exit status $? instead of 1" && exit 1 
echo ".................OK"

echo -n "test 03 - program exits without error: "
timeout 5 $PROG $PORT > output 2> /dev/null &
TO=$!
sleep 2
timeout 1 echo "hello world" | nc -4 127.0.0.1 $PORT -p $PORT_C
wait $TO
R=$?
[ "$R" == "124" ] && echo "KO -> program times out"           && exit 1
[ "$R" != "0" ]   && echo "KO -> exit status $R instead of 0" && exit 1
echo ".........OK"

echo -n "test 04 - program prints sender IP and port: "
IP=`cat output | head -n 1`
[ -z "$IP" ] && echo "KO -> nothing on stdout" && exit 1
echo "...OK"

echo -n "test 05 - printed IP is valid: "
IP=`cat output | head -n 1 | cut -d ' ' -f1`
[ "$IP" != "127.0.0.1" ] && echo "KO -> printed IP: $IP | expected: 127.0.0.1" && exit 1
echo ".................OK"

echo -n "test 06 - printed PORT is valid: "
PORT=`cat output | head -n 1 | cut -d ' ' -f2`
[ "$PORT" != "$PORT_C" ] && echo "KO -> printed PORT: $PORT | expected: $PORT_C" && exit 1
echo "...............OK"

echo -n "test 07 - program received a message: "
NB=`cat output | wc -l | tr -s ' ' | cut -d ' ' -f2`
[ "$NB" != "2" ] && echo "KO -> no message received" && exit 1
echo "..........OK"

echo -n "test 08 - received message is valid: "
MES=`cat output | tail -1`
[ "$MES" != "hello world" ] && echo "KO -> message received: $MES | expected: hello world" && exit 1
echo "...........OK"

echo -n "test 09 - check bind failure: "
timeout 5 nc -4l 127.0.0.1 $PORT > /dev/null &
TO=$!
sleep 1
$PROG $PORT 2> /dev/null && echo "KO -> exit status $R" && exit 1
wait $TO
echo "..................OK"

echo -n "test 10 - memory error: "
P=`which valgrind`
[ -z "$P" ] && echo "KO -> please install valgrind" && exit 1
valgrind --leak-check=full --error-exitcode=100 --log-file=valgrind.log $PROG $PORT > /dev/null &
V=$!
sleep 3
echo "hello world" | nc -4 -w1 127.0.0.1 $PORT
wait $V
[ "$?" == "100" ] && echo "KO -> memory pb please check valgrind.log" && exit 1
echo "........................OK"

rm output valgrind.log
