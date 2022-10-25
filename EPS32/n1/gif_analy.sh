num=$1
name=$2
cd /chwhsen/ICONS/gif
#gif下载与重命名
wget "https://developer.lametric.com/content/apps/icon_thumbs/$num.gif" -O $name.gif
#gif转bmp，方向调整
cd /chwhsen/ICONS/giftest_buffer
rm -f /chwhsen/ICONS/giftest_buffer/*
rm -f /chwhsen/ICONS/gif/*.log
convert /chwhsen/ICONS/gif/$name.gif $name.bmp
#查看色深
color_depth=$(xxd -i -ps -c 1 -s 28 -l 1 $name-0.bmp)
if [[ color_depth -eq 20 ]]
then
	#输出gif帧数
	num_bmp=$(ls -l | grep "^-" | wc -l)
	echo "This gif has $num_bmp pictures"
	#对每一帧进行解析
	for (( i=0;i<num_bmp;i++ ))
do
	convert -rotate -90  $name-$i.bmp $name--$i.bmp
	xxd -i /chwhsen/ICONS/giftest_buffer/$name--$i.bmp | grep '^  ' | sed 's/,//g' | sed  ':a;N;$!ba;s/\n//g' > /chwhsen/ICONS/gif/$name--$i.log
	echo -n "{\"data\":[" > /chwhsen/ICONS/gif/$name--${i}_json.log
	for ii in $(cat /chwhsen/ICONS/gif/$name--$i.log)
	do
		echo -n "{\"n\":$(($ii))}," >> /chwhsen/ICONS/gif/$name--${i}_json.log
	done
	echo -n "]}" >> /chwhsen/ICONS/gif/$name--${i}_json.log
	sed -i 's/,]/]/g' /chwhsen/ICONS/gif/$name--${i}_json.log
	aa=$(cat /chwhsen/ICONS/gif/$name--${i}_json.log)
	Offset_x=$(echo $aa | jq .data[10].n)
	color__x=$(echo $aa | jq .data[28].n)
	echo -n "{\"Type\":\"Gif\",\"Brightness\":2,\"totle\":$num_bmp,\"in\":$i,\"name\":\"$name\",\"bmp\":[" > /chwhsen/ICONS/gif/$name--${i}_bin.log
	for (( m=0;m<64;m++ ))
do
	echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $m * 4 + 2]].n)}," >> /chwhsen/ICONS/gif/$name--${i}_bin.log
	echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $m * 4 + 1]].n)}," >> /chwhsen/ICONS/gif/$name--${i}_bin.log
	echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $m * 4 + 0]].n)}," >> /chwhsen/ICONS/gif/$name--${i}_bin.log
done
	echo -n "]}" >> /chwhsen/ICONS/gif/$name--${i}_bin.log
	sed -i 's/,]/]/g' /chwhsen/ICONS/gif/$name--${i}_bin.log
done
	for (( i=0;i<num_bmp;i++ ))
	do
		mosquitto_pub -t "yucai21" -m $(cat /chwhsen/ICONS/gif/$name--${i}_bin.log)
	sleep 1s
	done
fi
