#!/bin/sh
  curl -X GET \
  https://api.todoist.com/rest/v2/tasks \
  -H "Authorization: Bearer $token" > /chwhsen/Big_Screen/source.log
num=$(cat /chwhsen/Big_Screen/source.log | jq -r '.[] | .content + "  " + .due.date' | grep `date "+%Y-%m-%d"` | wc -l)
echo -n "{\"Type\":\"Todo\",\"num\":$num,\"list\":[" > /chwhsen/Big_Screen/json.log
echo -n $(cat /chwhsen/Big_Screen/source.log | jq -r '.[] | "{\"no\":\"" + .content + "\",\"time\":\"" + .due.date + "\"},"' | grep `date "+%Y-%m-%d"` | sed ':a;N;$!ba;s/\n//g')  >> /chwhsen/Big_Screen/json.log
echo -n "]}" >> /chwhsen/Big_Screen/json.log
sed -i 's/,]/]/g' /chwhsen/Big_Screen/json.log
cp /chwhsen/Big_Screen/json.log /var/www/html/todo.list
