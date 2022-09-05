msg=$(curl "https://devapi.qweather.com/v7/astronomy/sun?key=${token}&location=${location}&date=`date "+%Y%m%d"`" | gunzip | jq .)
sun_rise_hour=$(echo $msg | jq -r .sunrise | sed -e 's/T0/:/g' -e 's/+/:/g' | awk -F ':' '{print $2}')
sun_rise_minute=$(echo $msg | jq -r .sunrise | sed -e 's/T/:/g' -e 's/+/:/g' | awk -F ':' '{print $3}')
sun_set_hour=$(echo $msg | jq -r .sunset | sed -e 's/T/:/g' -e 's/+/:/g' | awk -F ':' '{print $2}')
sun_set_minute=$(echo $msg | jq -r .sunset | sed -e 's/T/:/g' -e 's/+/:/g' | awk -F ':' '{print $3}')
mosquitto_pub -t inTopic -m "{\"Type\":\"SunTime\",\"R_H\":${sun_rise_hour},\"R_M\":${sun_rise_minute},\"S_H\":${sun_set_hour},\"S_M\":${sun_set_minute}}"
