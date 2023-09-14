#!/bin/bash
for ((i=1; i<=150; i++)); do
    random=$(od -An -N1 -i /dev/random)
    echo $random >> numbers.txt
done