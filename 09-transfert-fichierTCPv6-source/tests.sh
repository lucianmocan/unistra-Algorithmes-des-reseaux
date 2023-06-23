#!/bin/bash

PROG="./sender-tcp"
PORT=`shuf -i 10000-65000 -n 1`

echo -n "test 01 - program without arg: "
$PROG > /dev/null 2>&1 && echo "KO -> exit status $? instead of 1" && exit 1 
echo "....................OK"

echo -n "test 02 - program with only one arg: "
$PROG $PORT 2> /dev/null && echo "KO -> exit status $? instead of 1" && exit 1 
echo "..............OK"

echo -n "test 03 - invalid port number: "
$PROG 65001 sender-tcp > /dev/null 2>&1 && echo "KO -> exit status $? instead of 1" && exit 1 
echo "....................OK"

echo -n "test 04 - program with unknown filename: "
$PROG $PORT abcd 2> /dev/null && echo "KO -> exit status $? instead of 1" && exit 1 
echo "..........OK"

echo -n "test 05 - program exits without error: "
timeout 5 nc -6l ::1 $PORT > copy.tmp &
TO=$!
sleep 2
! $PROG $PORT sender-tcp && echo "KO -> exit status $?" && exit 1
wait $TO
echo "............OK"

echo -n "test 06 - program sent some data: "
[ -s "copy.tmp" ] || (echo "KO -> no data (empty file)" && exit 1)
echo ".................OK"

echo -n "test 07 - files are the same: "
cmp sender-tcp copy.tmp 2> /dev/null || (echo "KO -> files differ" && exit 1)
echo ".....................OK"

echo -n "test 08 - connection to a non listening server: "
$PROG $PORT sender-tcp 2> /dev/null && echo "KO -> exit status $?" && exit 1 
echo "...OK"

echo -n "test 09 - memory error: "
P=`which valgrind`
[ -z "$P" ] && echo "KO -> please install valgrind" && exit 1
timeout 5 nc -6l ::1 $PORT > copy.tmp &
sleep 2
valgrind --leak-check=full --error-exitcode=100 --log-file=valgrind.log $PROG $PORT sender-tcp > /dev/null
[ "$?" == "100" ] && echo "KO -> memory pb please check valgrind.log" && exit 1
echo "...........................OK"

rm copy.tmp valgrind.log
