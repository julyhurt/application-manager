# application-manager
manage applications (long running and short running) running on a host

# Build environment
sudo apt-get install openssh-server g++ gdb gdbserver

# third party used in code
apt-get install make libboost-all-dev libssl-dev libboost-all-dev libcpprest-dev libjsoncpp-dev libace-dev libgoogle-glog-dev

# RPM install on ubuntu dependency
apt-get install alien

# RPM package build dependency
apt-get install ruby ruby-dev rubygems

gem install fpm




# Installation command on ubuntu
sudo rpm -ivh ./Application-Manager-1.0-1.x86_64.rpm --nodeps
