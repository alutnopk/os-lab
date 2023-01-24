#!/bin/bash

if [ $# -lt 3 ]; then
    echo "Usage: $0 source_dir dest_dir attribute1 attribute2 ..."
    exit 1
fi

source_dir=$1
dest_dir=$2

shift 2

attributes=("$@")

if [ ! -d $dest_dir ]; then
    mkdir $dest_dir
fi

for file in $source_dir/*.jsonl; do
    
    csv_file="$dest_dir/$(basename $file .jsonl).csv"

    jq_attributes=""
    for attribute in "${attributes[@]}"; do
        jq_attributes+=" .$attribute,"
    done
    jq_attributes="${jq_attributes%?}"
    
    jq -r -c "select([$jq_attributes] | all(.; . != null)) | [$jq_attributes] | @csv" $file > $csv_file
    
    echo ${attributes[*]} | tr ' ' ',' > $csv_file.tmp
    cat $csv_file >> $csv_file.tmp
    mv $csv_file.tmp $csv_file
done
