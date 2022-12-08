#!/bin/bash
time=$(date "+%H:%M")
hour=$(date "+%H")
minute=$(date "+%M")
second=$(date "+%S")
echo $time
if [ "23:27"a == ${time}a ];
then
curl "https://devapi.qweather.com/v7/astronomy/sun?location=-------&key=-------------&gzip=n&date=`date "+%Y%m%d"`" > /chwhsen/xuanGuan/XuanLed.log
else
echo "Nothing"
fi
RiseTime=$(cat /chwhsen/xuanGuan/XuanLed.log | jq -r .sunrise | sed 's/+/T/g' |awk  -F 'T' '{print $2}')
SetTime=$(cat /chwhsen/xuanGuan/XuanLed.log | jq -r .sunset | sed 's/+/T/g' |awk  -F 'T' '{print $2}')
if [ ${RiseTime}a == ${time}a ];
then
mosquitto_pub -h chwhsen.xyz -t EspIn -m '{"Type":"Light","light":0}'
echo "关灯"
elif [ ${SetTime}a == ${time}a ];
then
mosquitto_pub -h chwhsen.xyz -t EspIn -m '{"Type":"Light","light":1}'
echo "开灯"
else
mosquitto_pub -h chwhsen.xyz -t EspIn -m "{\"Type\":\"time\",\"hour\":$hour,\"minute\":$minute,\"second\":$second}"
fi
