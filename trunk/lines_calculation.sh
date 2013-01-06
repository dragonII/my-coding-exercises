#!/bin/sh
total=0
for i in `find ./ -name "*.c" -exec wc -l {} \; | awk '{print $1}' `
    do
        #echo "total=$total + $i"
        total=$(echo "$total + $i" | bc)
    done
echo "total C=$total"

total=0
for i in `find ./ -name "*.h" -exec wc -l {} \; | awk '{print $1}' `
    do
        #echo "total=$total + $i"
        total=$(echo "$total + $i" | bc)
    done
echo "total H=$total"

