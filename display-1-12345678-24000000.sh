#!/bin/sh

INFILE=$(ls -1 /mpidata/ergebnisse/g15_twitter.data.out.* | head -n1)

for i in 1 2 12345677 12345678 12345679 23999999 24000000
do
  echo "\nTweet mit Relevanz $i:"
  sed -n "$i p" ${INFILE} |
      (read a; rank=$(echo $a| cut -d" " -f1) ; grep "^$a " /mpidata/parsys14/gross/twitter.data.$rank)
done
echo "\ndas Suchwort war \"app\"\n"

