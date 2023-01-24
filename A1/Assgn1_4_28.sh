#!/bin/bash
file_path=$1
keyword=$2
while IFS='' read -r line || [[ -n "$line" ]];do
  if grep -q $keyword <<< $line; then
    echo "$line" | tr '[A-Z]' '[a-z]' | sed 's/[a-z][^a-z\n]*[a-z]\?/\u\0/g'
  else
    echo "$line"
  fi
done < "$file_path"
