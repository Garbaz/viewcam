#!/bin/bash
devices=($(ls /dev/video* 2>/dev/null)) || exit 1
c="0"
for d in "${devices[@]}";do
echo "$c) $d"
c=$(($c + 1))
done
printf ">"
read $s
selection=$( printf "${devices[$s]}" | sed 's/\/dev\/video//g')
$(dirname $0)/viewcam $selection
