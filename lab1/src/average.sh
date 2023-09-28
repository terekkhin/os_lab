#!/bin/bash
if [[ $# -eq 0 ]]; then
    echo "Нет аргументов"
    else
        count=$#
        sum=0
        for i in $@; do
            sum=$(($sum+$i))
        done
        echo "$count" 
	echo "scale=2; $sum/$count" | bc -l
fi

