#!/bin/bash
count=0
num=0
while [ -n "$1" ]
do
count=$[ $count + 1 ]
num=$[ $num + $1]
shift
done
avr=$(bc<<<"scale=2;$num/$count")
echo "Количество = $count"
echo "Среднее арифметическое = $avr"