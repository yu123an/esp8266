num=$1
name=$2
cd /chwhsen/ICONS/gif
wget "https://developer.lametric.com/content/apps/icon_thumbs/${num}.gif" -O ${name}.gif
cd /chwhsen/ICONS/gif_buffer
convert /chwhsen/ICONS/gif/${name}.gif ${name}.bmp
num_bmp=$(ls -l | grep "^-" | wc -l)
for (( i=0;i<num_bmp;i++ ))
do
convert -rotate -90  ${name}-${i}.bmp ${name}-${i}.bmp

xxd -i /chwhsen/ICONS/gif_buffer/${name}-${i}.bmp | grep '^  ' | sed 's/,//g' | sed  ':a;N;$!ba;s/\n//g' > /chwhsen/ICONS/icon_gif.log
echo -n "{\"data\":[" > /chwhsen/ICONS/icon_gif_bin.log
for ii in $(cat /chwhsen/ICONS/icon_gif.log)
do
echo -n "{\"n\":$(($ii))}," >> /chwhsen/ICONS/icon_gif_bin.log
done
echo -n "]}" >> /chwhsen/ICONS/icon_gif_bin.log
sed -i 's/,]/]/g' /chwhsen/ICONS/icon_gif_bin.log
aa=$(cat /chwhsen/ICONS/icon_gif_bin.log)
#读取头文件关键词
Offset_x=$(echo $aa | jq .data[10].n)
image_x=$(echo $aa | jq .data[18].n)
image_y=$(echo $aa | jq .data[22].n)
color__x=$(echo $aa | jq .data[28].n)
#RGB取值重组
echo -n "{\"Type\":\"Gif\",\"Brightness\":2,\"totle\":${num_bmp},\"in\":${i},\"name\":\"${name}\",\"bmp\":[" > /chwhsen/ICONS/icon_gif_bin.log
for (( m=0;m<64;m++ ))
#for i in $(seq 0 64)
do 
echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $m * 4 + 2]].n)}," >> /chwhsen/ICONS/icon_gif_bin.log
echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $m * 4 + 1]].n)}," >> /chwhsen/ICONS/icon_gif_bin.log
echo -n "{\"n\":$(echo $aa | jq .data[$[$Offset_x + $m * 4 + 0]].n)}," >> /chwhsen/ICONS/icon_gif_bin.log
done
echo -n "]}" >> /chwhsen/ICONS/icon_gif_bin.log
sed -i 's/,]/]/g' /chwhsen/ICONS/icon_gif_bin.log
mosquitto_pub -t "test" -m $(cat /chwhsen/ICONS/icon_gif_bin.log)
done
rm -f /chwhsen/ICONS/gif_buffer
