#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 filename"
    exit 1
fi

input_file="$1"
output_file="converted.packet"

touch "$output_file"

tr -d '\r' < "$input_file" > temp_file
perl -pe 's/\r\n|\r/\n/g; s/\n/\r\n/g' "$input_file" > "$output_file"

echo "Converted file saved as $output_file"
