version=1.0

all:
	cd ApplicationManager; make
	cd CommandLine; make
	make build_dir
	make deb

build_dir:
	rm -rf release
	mkdir -p release/script
	cp ./CommandLine/appc ./release
	cp ./ApplicationManager/appsvc ./release
	cp ./ApplicationManager/appsvc.json ./release
	cp ./script/*.sh ./release/script
	chmod +x ./release/script/*.sh
	
deb:
	if [ ! -d "/opt/appmanager" ]; then \
		mkdir /opt/appmanager; \
	fi
	rm -rf /opt/appmanager/*
	cp -rf ./release/* /opt/appmanager
	fpm -s dir -t deb -v ${version} -n application-namager --post-install /opt/appmanager/script/install_ubuntu.sh --before-remove /opt/appmanager/script/pre_uninstall_ubuntu.sh --after-remove /opt/appmanager/script/uninstall_ubuntu.sh /opt/appmanager/
	
install:
	dpkg -i application-namager_${version}_amd64.deb
	
uninstall:
	dpkg -P application-namager

clean:
	cd CommandLine; make clean
	cd ApplicationManager; make clean
	rm -rf release
	rm -f *.deb
