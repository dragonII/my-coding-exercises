#!/bin/sh
c_total=0
for i in `find ./ -name "*.c" -exec wc -l {} \; | awk '{print $1}' `
    do
        c_total=$(echo "$c_total + $i" | bc)
    done
echo "total   C=$c_total"

h_total=0
for i in `find ./ -name "*.h" -exec wc -l {} \; | awk '{print $1}' `
    do
        h_total=$(echo "$h_total + $i" | bc)
    done
echo "total   H=$h_total"

total=`expr $c_total + $h_total`
echo "total ALL=$total"
