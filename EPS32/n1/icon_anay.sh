num=$1
name=$2
cd /chwhsen/ICONS
wget "https://developer.lametric.com/content/apps/icon_thumbs/${num}.png" -O ${name}.png
convert ${name}.png ${name}.bmp
convert -rotate -90  ${name}.bmp ${name}.bmp
#aa=`xxd -i  $num.bmp | sed -e "s/unsigned char __${num}_bmp\[\] =//g" -e "s/unsigned int __${num}_bmp_len = 394;//g" -e 's/{/(/g' -e 's/}/)/g'  -e 's/,/ /g' -e 's/;//g' -e 's/(  /(/g' | sed  ':a;N;$!ba;s/\n//g' | sed 's/(  /(/g'`
#echo ${aa[2]}
