#!/bin/bash
apppath=/opt/appmanager
if [ ! -d $apppath ];then
	mkdir $apppath
fi

service appsvc stop
sleep 2

#tar zxvf appmg.tar.gz -C $apppath
cp -f $apppath/script/appmg_service.sh /etc/init.d/appsvc
chmod 755 /etc/init.d/appsvc
chmod 757 /opt

#sudo update-rc.d appmg defaults

systemctl enable appsvc
service appsvc stop
sleep 2
service appsvc start

rm -rf /usr/bin/appc
ln -s /opt/appmanager/appc /usr/bin/appc
