version=1.0

all:
	cd ApplicationManager; make
	cd CommandLine; make
	make build_dir
	make rpm

build_dir:
	rm -rf release
	mkdir -p release/script
	cp ./CommandLine/appc ./release
	cp ./ApplicationManager/appsvc ./release
	cp ./ApplicationManager/appsvc.json ./release
	cp ./script/*.sh ./release/script
	chmod +x ./release/script/*.sh
	
rpm:
	if [ ! -d "/opt/appmanager" ]; then \
		mkdir /opt/appmanager; \
	fi
	cp -rf ./release/* /opt/appmanager
	fpm -s dir -t rpm -v ${version} -n Application-Manager --post-install /opt/appmanager/script/install_ubuntu.sh --before-remove /opt/appmanager/script/pre_uninstall_ubuntu.sh --after-remove /opt/appmanager/script/uninstall_ubuntu.sh /opt/appmanager/
	
install:
	rpm -ivh Application-Manager-${version}-1.x86_64.rpm --nodeps
	
uninstall:
	rpm -e Application-Manager-${version}-1.x86_64

clean:
	cd CommandLine; make clean
	cd ApplicationManager; make clean
	rm -rf release
	rm -f *.rpm
