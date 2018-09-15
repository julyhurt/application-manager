#! /bin/sh
PROG_NAME="appsvc"
PROG_PATH="/opt/appmanager" 
PROG_ARGS="" 
LOG_FILE="/var/log/appsvc_watchdog.log"

start_appsvc() {
	cd $PROG_PATH
	## Change from /dev/null to something like /var/log/$PROG if you want to save output.
    $PROG_PATH/$PROG_NAME $PROG_ARGS &
	log "start appsvc..."
}
log(){
        echo "[`date`]""$1" >> ${LOG_FILE}
}

start_appsvc
while true ; do
     APPSVC_INSTENCE_NUM=`ps aux | grep -w ${PROG_PATH}/${PROG_NAME} | grep -v grep |wc -l`
     echo $APPSVC_INSTENCE_NUM
     if [ "${APPSVC_INSTENCE_NUM}" -lt "1" ];then
         log "${PROG_NAME} was killed."		 
         start_appsvc
    elif [ "${APPSVC_INSTENCE_NUM}" -gt "1" ];then
        log "more than 1 ${PROG_NAME},killall ${PROG_NAME}."
        killall -9 $PROG_NAME
        start_appsvc
     fi
     appsvc_STAT=`ps aux | grep -w ${PROG_PATH}/${PRO_NAME} | grep T | grep -v grep | wc -l`
     if [ "${appsvc_STAT}" -gt "0" ];then
	     log "zombie ${PROG_NAME},killall ${PROG_NAME}."
         killall -9 ${PROG_NAME}
         start_appsvc
    fi
     sleep 5s
 done
 
 exit 0
