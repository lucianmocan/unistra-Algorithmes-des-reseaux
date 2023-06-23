#!/bin/bash

PROG="./receiver-tcp"
PORT=`shuf -i 10000-65000 -n 1`

echo -n "test 01 - program without arg: "
$PROG > /dev/null 2>&1 && echo "KO -> exit status $? instead of 1" && exit 1 
echo "....................OK"

echo -n "test 02 - invalid port number: "
$PROG 65001 > /dev/null 2>&1 && echo "KO -> exit status $? instead of 1" && exit 1 
echo "....................OK"

echo -n "test 03 - program exits without error: "
timeout 5 $PROG $PORT 2> /dev/null &
TO=$!
sleep 1
timeout 1 nc -6 ::1 $PORT < receiver-tcp &
wait $!
wait $TO
R=$?
[ "$R" == "124" ] && echo "KO -> program times out"           && exit 1
[ "$R" != "0" ]   && echo "KO -> exit status $R instead of 0" && exit 1
echo "............OK"

echo -n "test 04 - file copy.tmp exists: "
[ ! -f "copy.tmp" ] && echo "KO -> file does not exist" && exit 1
echo "...................OK"

echo -n "test 05 - files are the same: "
cmp receiver-tcp copy.tmp 2> /dev/null || (echo "KO -> files differ" && exit 1)
echo ".....................OK"

echo -n "test 06 - check bind failure: "
timeout 5 nc -6l ::1 $PORT &
TO=$!
sleep 1
$PROG $PORT 2> /dev/null && echo "KO -> exit status $? instead of 1" && exit 1 
wait $TO
echo ".....................OK"

echo -n "test 07 - check open failure: "
chmod 000 copy.tmp
$PROG $PORT 2> /dev/null &
PID=$!
timeout 1 nc -6 ::1 $PORT < receiver-tcp &
wait $!
wait $PID
R=$?
[ "$R" != "1" ] && echo "KO -> exit status $R instead of 1" && exit 1
chmod 644 copy.tmp
echo ".....................OK"

echo -n "test 08 - memory error: "
P=`which valgrind`
[ -z "$P" ] && echo "KO -> please install valgrind" && exit 1
valgrind --leak-check=full --error-exitcode=100 --log-file=valgrind.log $PROG $PORT > /dev/null &
V=$!
sleep 2
cat receiver-tcp | nc -6 -w1 ::1 $PORT
wait $V
[ "$?" == "100" ] && echo "KO -> memory pb please check valgrind.log" && exit 1
echo "...........................OK"

rm copy.tmp valgrind.log
