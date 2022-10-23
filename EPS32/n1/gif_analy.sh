num=$1
name=$2
cd /chwhsen/ICONS/gif
#gif下载与重命名
wget "https://developer.lametric.com/content/apps/icon_thumbs/${num}.gif" -O ${name}.gif
#gif转bmp，方向调整
cd /chwhsen/ICONS/giftest_buffer
rm -f /chwhsen/ICONS/giftest_buffer/*
convert /chwhsen/ICONS/gif/${name}.gif ${name}.bmp
#输出gif帧数
num_bmp=$(ls -l | grep "^-" | wc -l)
echo "This gif has $num_bmp pictures"
#对每一帧进行解析
for (( i=0;i<num_bmp;i++ ))
do
convert -rotate -90  ${name}-${i}.bmp ${name}--${i}.bmp

xxd -i /chwhsen/ICONS/giftest_buffer/${name}--${i}.bmp | grep '^  ' | sed 's/,//g' | sed  ':a;N;$!ba;s/\n//g' > /chwhsen/ICONS/gif/${name}--${i}.log
echo -n "{\"data\":[" > /chwhsen/ICONS/gif/${name}--${i}_json.log
for ii in $(cat /chwhsen/ICONS/gif/${name}--${i}.log)
do
echo -n "{\"n\":$(($ii))}," >> /chwhsen/ICONS/gif/${name}--${i}_json.log
done
echo -n "]}" >> /chwhsen/ICONS/gif/${name}--${i}_json.log
sed -i 's/,]/]/g' /chwhsen/ICONS/gif/${name}--${i}_json.log
aa=$(cat /chwhsen/ICONS/gif/${name}--${i}_json.log)
color__x=$(echo $aa | jq .data[28].n)
#判断色深，只处理32位bmp文件，对于其他文件进行跳过处理
if [[ color__x -eq 24 ]]
then
echo " I am good !!!"
else
echo "I am so bed....."
break
fi
done
#对其中前两帧进行解析读取头文件信息
for (( m=0;m<2;m++ ))
do
aa=$(cat /chwhsen/ICONS/gif/${name}--${m}_json.log)
color__x=$(echo $aa | jq .data[28].n)
echo "No:$m ;The color depth is $color__x"
done
xxd /chwhsen/ICONS/giftest_buffer/${name}--0.bmp
xxd /chwhsen/ICONS/giftest_buffer/${name}--1.bmp
