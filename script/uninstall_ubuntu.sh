#!/bin/bash
service appsvc stop
systemctl disable appsvc
rm -rf /opt/appmanager

rm -f /usr/bin/appc
rm -f /etc/init.d/appmg
