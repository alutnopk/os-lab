#!/bin/bash
awk '{depts[$2]++} END {for(dept in depts) print dept, depts[dept]}' "$1" | sort -k2nr -k1
echo
awk '{studs[$1]++} END{for(stu in studs){if(studs[stu]>1)print stu; if(studs[stu]==1) count++;}print count}' "$1"
