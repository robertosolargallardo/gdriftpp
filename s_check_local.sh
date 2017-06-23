#!/bin/bash

# Primera version de check de servicios
# Esta version depende del nombre de los servicios, el numero de controllers, y de las rutas absolutas
# Por ahora eso implica que tambien depende del usuario

# Check simple de proceso (retorna el numero de procesos corriendo dado el nombre)
check_process() {
	echo "$ts: checking $1"
	[ "$1" = "" ]  && return 0
#	[ `pgrep -f $1 | wc -l` ] && return 1 || return 0
	return `pgrep -f $1 | wc -l`
}

# Check + Restart, usa check_process para revisar si el servicio esta corriendo
# Si la prueba falla, termina el screen (si lo encuentra) y relanza el servicio en un nuevo screen
check_restart() {
	check_process "gdrift_service_${1}"
	CHECK_RET=$?
	if [ $CHECK_RET -eq 0 ];
	then
		echo "$ts: Not running, restarting"
		killall screen_${1}
		sleep 1
		# Notar que estoy agregando $2 a todos los comandos
		# Como solo entrego ese parametro para controller, los demas usan null y funciona
		screen -dmS screen_${1} /home/vsepulve/gdrift_services/build/bin/gdrift_service_${1} /home/vsepulve/gdrift_services/hosts_local.json ${2}
		restore=1
	else
		echo "$ts: Service Ok"
	fi
	sleep 1
}

# El while... sleep es solo mientras lo pruebo
# En la aplicacion real estara asociado al crontab
#while [ 1 ]; do 

# timestamp
ts=`date +%T`
restore=0

check_restart "analyzer" 
echo "-----"
check_restart "controller" "0"
echo "-----"
check_restart "scheduler" 
echo "-----"

if [ $restore -eq 1 ];
then
	# send restore command
	echo "Sending Restore Command"
	curl -H "Content-Type: application/json" -X POST http://localhost:2002/scheduler --data-binary "@/home/vsepulve/gdrift_services/test/restore.json"
	echo "-----"
fi

#sleep 3
#done





