#!/bin/bash
msg=$( curl "https://devapi.qweather.com/v7/weather/24h?key=${token}&location=${key}&lang=en" | gunzip | jq .hourly[0])
temp=$(echo $msg | jq -r .temp)
wind=$(echo $msg | jq -r .windDir)
humidity=$(echo $msg | jq -r .humidity)
name=wind_${wind}.bmp
xxd -i /chwhsen/ICONS/$name | grep '^  ' | sed 's/,//g' | sed  ':a;N;$!ba;s/\n//g' > /chwhsen/ICONS/icon.log
echo -n "{\"data\":[" > /chwhsen/ICONS/icon_bin.log
for ii in $(cat /chwhsen/ICONS/icon.log)
do
echo -n "{\"n\":$(($ii))}," >> /chwhsen/ICONS/icon_bin.log
done
echo -n "]}" >> /chwhsen/ICONS/icon_bin.log
sed -i 's/,]/]/g' /chwhsen/ICONS/icon_bin.log
aa=$(cat /chwhsen/ICONS/icon_bin.log)
#读取头文件关键词
Offset_x=$(echo $aa | jq .data[10].n)
image_x=$(echo $aa | jq .data[18].n)
image_y=$(echo $aa | jq .data[22].n)
color__x=$(echo $aa | jq .data[28].n)
#RGB取值重组
echo -n "{\"Type\":\"Temp\",\"temp\":${temp},\"hump\":${humidity},\"bmp\":[" > /chwhsen/ICONS/icon_bin.log
for (( i=0;i<64;i++ ))
#for i in $(seq 0 64)
do 
echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $i * 4 + 2]].n)}," >> /chwhsen/ICONS/icon_bin.log
echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $i * 4 + 1]].n)}," >> /chwhsen/ICONS/icon_bin.log
echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $i * 4 + 0]].n)}," >> /chwhsen/ICONS/icon_bin.log
done
echo -n "]}" >> /chwhsen/ICONS/icon_bin.log
sed -i 's/,]/]/g' /chwhsen/ICONS/icon_bin.log
mosquitto_pub -t "inTopic" -m $(cat /chwhsen/ICONS/icon_bin.log)


#mosquitto_pub -t "inTopic" -m "{\"Type\":\"Temp\",\"temp\":${temp},\"hump\":${humidity}}"
