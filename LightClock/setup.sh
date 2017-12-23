#!/bin/sh

rfcomm bind rfcomm0 20:16:05:23:23:39

time=`date +%s`
tz=$((3600*3))
hour=6 #`date +%-H`
min=0 #`date +%-M`
sec=0 #`date +%-S`
timeStart=$(($hour*3600+$min*60+$sec))
timeStop=$(($timeStart+20*60))
echo 'Time: '$(($time+$tz))'\n' >/dev/rfcomm0
echo 'Light: 0 '$timeStart' '$timeStop' 62\n' >/dev/rfcomm0

rfcomm release rfcomm0 20:16:05:23:23:39
