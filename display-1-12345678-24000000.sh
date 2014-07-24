#!/bin/sh

for i in 1 2 12345677 12345678 12345679 23999999 24000000
do
  echo "\nTweet mit Relevanz $i:"
  sed -n "$i p" /mpidata/ergebnisse/g15_twitter.data.out.0 |
      (read a; grep "^$a " /mpidata/parsys14/gross/twitter.data.0)
done
echo "\ndas Suchwort war \"app\"\n"

