#!/bin/bash

file_path=$1
keyword=$2

while IFS='' read -r line || [[ -n "$line" ]]; do
  if grep -q $keyword <<< $line; then
    echo "fOUND: $line" | tr 'a-zA-Z' 'A-Za-z'
  else
    echo "Not Found: $line"
  fi
done < "$file_path"
