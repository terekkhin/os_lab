#!/bin/bash
rm numbers.txt 2> /dev/null
touch numbers.txt
for ((i=1; i<=150; i++)); do
	echo $(od -An -N1 -i /dev/random) >> numbers.txt
done
