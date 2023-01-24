#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Please give one valid path of folder"
	exit 1
fi

given_dir=$1

for file in $(find "$given_dir" -name "*.py" -type f); do
	echo "FILE: ${file##*/}	 PATH: $file"

	line_number=0
    while IFS='' read -r line || [[ -n "$line" ]]; do
        ((line_number++))
        if [[ $line == *[#]* ]]; then
            echo "Line $line_number: $line"
        elif [[ $line == ^\"\"\" ]]; then
            echo "Line $line_number: $line"
            while IFS='' read -r line || [[ -n "$line" ]]; do
                ((line_number++))
                if [[ $line == ^\"\"\" ]]; then
                    break
                else
                    echo "LINE: $line_number: $line"
                fi
            done
        fi
	done < "$file"
done
