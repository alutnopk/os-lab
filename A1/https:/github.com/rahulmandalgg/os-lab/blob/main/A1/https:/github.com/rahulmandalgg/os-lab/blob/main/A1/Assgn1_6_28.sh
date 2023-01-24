#!/bin/bash

file="/home/astitva27/Documents/OS_Lab/os_ass1/q6/chhota.txt"
while IFS= read line
do
    num=$(echo $line | grep -o '[0-9]\+')
    arr=()
    arr[0]=0
    arr[1]=0
    for((i=2;i<=num;i++)); do
        arr[i]=1
    done
    for((i=2;i*i<=num;i++)); do
        if [ ${arr[i]} == 1 ]
        then
            for((j=i*i;j<=num;j+=i)); do
                arr[j]=0
            done
        fi
    done
    for((i=2;i<=num;i++)); do
        if [ ${arr[i]} == 1 ]
        then
            echo -n $i
        fi
    done
    echo ""
done <"$file"
