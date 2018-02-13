ps -ef|grep dezhoukvredis| grep -v grep|awk '{print $2}'|xargs kill -9
