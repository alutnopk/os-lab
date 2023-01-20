#!/bin/bash
invalid_words=($(cat fruits.txt))
while IFS= read -r line;do
    if [[ ${line} =~ ^[a-zA-Z][a-zA-Z0-9]{4,19}$&&"${line}" =~ [0-9] ]]; then
        for word in "${invalid_words[@]}";do
            if grep -iqF $word<<<$line;then
                echo "NO">>validation_results.txt
                continue 2;
            fi
        done
    		echo "YES">>validation_results.txt
    else	
        	echo "NO">>validation_results.txt
    fi
done<usernames.txt

