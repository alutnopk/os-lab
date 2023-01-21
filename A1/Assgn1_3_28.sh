#!/bin/bash

# usage: script.sh jsonl_dir csv_dir attr1 attr2 ...

jsonl_dir=$1
csv_dir=$2
shift 2
attributes="$@"

# replace all spaces with commas and prefix each word with a dot
#string="Hello World"

# prefix each word with a dot, and replace all spaces with commas
#new_string=$(echo $string | sed -r 's/ /, ./g; s/^/./')




[ ! -e "$csv_dir" ] && mkdir "$csv_dir"
# loop through all the JSONL files in the given directory
for jsonl_file in "$jsonl_dir"/*.jsonl; do
  # create a corresponding CSV file with the same name in the output directory
  csv_file="$csv_dir"/$(basename "$jsonl_file" .jsonl).csv
  attrs=$(echo $attributes | sed -r 's/ /, ./g; s/^/./') #s/$/\n/ # sed -r 's/ /,", ",./g; s/^/./'
  [ ! -e "$csv_file" ] && { touch "$csv_file"; echo "$attrs" > "$csv_file"; }
  echo $attrs
  # extract the specified attributes from each JSON object
  # cat "$jsonl_file" | jq -r '.' | jq -r '.'$attributes | sed 's/"/""/g' | awk -F, '{gsub(/"/,"\"\""); print}' | grep -v '^\s*$' | sed 's/^,//' >> "$csv_file"
  cat "$jsonl_file" | jq -c "$attrs">> "$csv_file" # 

	# while IFS='\n' read -r line || [[ -n "$line" ]]; do
	# 		echo "$line"
	# done < "$json1_file"


done
