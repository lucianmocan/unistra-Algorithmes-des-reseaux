#!/bin/bash

PROG="./receiver-udp"
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
echo "hello world" | nc -6u -w1 ::1 $PORT -p $PORT_C
wait $TO
R=$?
[ "$R" == "124" ] && echo "KO -> program times out"           && exit 1
[ "$R" != "0" ]   && echo "KO -> exit status $R instead of 0" && exit 1
echo ".........OK"

echo -n "test 04 - program received a message: "
MES=`cat output | head -n 1`
[ -z "$MES" ] && echo "KO -> no message received" && exit 1
echo "..........OK"

echo -n "test 05 - message is valid: "
[ "$MES" != "hello world" ] && echo "KO -> message received: $MES | expected: hello world" && exit 1
echo "....................OK"

echo -n "test 06 - program prints sender IP and port: "
NB=`cat output | wc -l | tr -s ' ' | cut -d ' ' -f2`
[ "$NB" != "2" ] && echo "KO -> nothing on stdout" && exit 1
echo "...OK"

echo -n "test 07 - printed IP is valid: "
IP=`cat output | tail -1 | cut -d ' ' -f1`
[ "$IP" != "::1" ] && echo "KO -> printed IP: $IP | expected: ::1" && exit 1
echo ".................OK"

echo -n "test 08 - printed PORT is valid: "
PORT_P=`cat output | tail -1 | cut -d ' ' -f2`
[ "$PORT_P" != "$PORT_C" ] && echo "KO -> printed PORT: $PORT_P | expected: $PORT_C" && exit 1
echo "...............OK"

echo -n "test 09 - check bind failure: "
timeout 5 nc -6ul ::1 $PORT & > /tmp/output
TO=$!
sleep 2
timeout 1 $PROG $PORT 2> /dev/null
R="$?"
[ "$R" == "124" ] && echo "KO -> program times out"           && exit 1
[ "$R" != "1" ]   && echo "KO -> exit status $R instead of 1" && exit 1
wait $TO
echo "..................OK"

echo -n "test 10 - memory error: "
P=`which valgrind`
[ -z "$P" ] && echo "KO -> please install valgrind" && exit 1
valgrind --leak-check=full --error-exitcode=100 --log-file=valgrind.log $PROG $PORT > /dev/null &
V=$!
sleep 3
echo "hello world" | nc -6u -w1 ::1 $PORT
wait $V
[ "$?" == "100" ] && echo "KO -> memory pb please check valgrind.log" && exit 1
echo "........................OK"

rm output valgrind.log
