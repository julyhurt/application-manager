
# Application Manager
Application manager is a daemon process running on host that can manage all applications (long running and short running), provide cli to view and control applications.

||Tecnical|
|:-|:-|
|1|C++11|
|2|boost|
|3|cpprestsdk|
|4|ACE|
|5|glog|
|6|jsoncpp|


## Setup build environment
```javascript
sudo apt-get install openssh-server g++ gdb gdbserver
sudo apt-get install make libboost-all-dev libssl-dev libcpprest-dev libjsoncpp-dev libace-dev libgoogle-glog-dev
```

## RPM package build dependency
```javascript
sudo apt-get install ruby ruby-dev rubygems
sudo gem install fpm
```



## Installation command on ubuntu
```javascript
sudo dpkg application-namager_1.0_amd64.deb
```
