#!/bin/bash

PROG="./client-http"

echo -n "test 01 - program without arg: "
$PROG > /dev/null 2>&1 && echo "KO -> exit status $? instead of 1" && exit 1 
echo "...................OK"

echo -n "test 02 - connection to a non existing server: "
$PROG fdjkfslkfj 2> /dev/null && echo "KO -> exit status $?" && exit 1  
echo "...OK"

echo -n "test 03 - program exits without error: "
timeout 5 $PROG ip6.me > /dev/null
R=$?
[ "$R" == "124" ] && echo "KO -> program times out"           && exit 1
[ "$R" != "0" ]   && echo "KO -> exit status $R instead of 0" && exit 1
echo "...........OK"

echo -n "test 04 - IPv4 html response is valid: "
$PROG ip4.me > output 2> /dev/null
tail -n 30 output > pg.http
echo -e "GET / HTTP/1.1\r\nHost: ip4.me\r\nConnection:close\r\n\r\n" | nc -4 ip4.me 80 > output
tail -n 30 output > legacy.http
cmp legacy.http pg.http 2> /dev/null || { echo "KO -> files differ" && exit 1; }
echo "...........OK"

echo -n "test 05 - IPv6 html response is valid: "
$PROG ip6only.me > output 2> /dev/null
tail -n 30 output > pg.http
echo -e "GET / HTTP/1.1\r\nHost: ip6only.me\r\nConnection:close\r\n\r\n" | nc -6 ip6only.me 80 > output
tail -n 30 output > legacy.http
cmp legacy.http pg.http 2> /dev/null || { echo "KO -> files differ" && exit 1; }
echo "...........OK"

echo -n "test 06 - memory error: "
P=`which valgrind`
[ -z "$P" ] && echo "KO -> please install valgrind" && exit 1
valgrind --leak-check=full --error-exitcode=100 --log-file=valgrind.log $PROG google.com > /dev/null
[ "$?" == "100" ] && echo "KO -> memory pb please check file valgrind.log" && exit 1
echo "..........................OK"

rm output legacy.http pg.http valgrind.log
