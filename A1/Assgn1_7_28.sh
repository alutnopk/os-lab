#!/bin/bash
if [ $# -eq 0 ]; then
  echo "Please provide an input directory path"
  exit 1
fi
input_dir=$1
output_dir=$2
if [ ! -d "$output_dir" ]; then
  mkdir "$output_dir"
fi
for letter in {a..z};do
outputFile=$2"/"$letter".txt"
if [ ! -f outputFile ]; then
            `touch $outputFile`
fi
done
for file in "$input_dir"/*.txt; do
  while IFS= read -r line; do
    first_letter=$(echo "$line" | head -c 1)
    echo "$line" >> "$output_dir/$first_letter.txt"
  done < "$file"
done
for file in "$output_dir"/*.txt; do
  sort "$file" -o "$file"
done
echo "Names sorted and grouped by first letter in output directory"
