#!/bin/bash

# Primera version de check de servicios
# Esta version depende del nombre de los servicios, el numero de controllers, y de las rutas absolutas
# Por ahora eso implica que tambien depende del usuario
# Para agregar el ceck al crontab, agregar el comando (reemplazando el usuario y las rutas) a crontab :
# */5 * * * * vsepulve /home/vsepulve/gdrift_services/s_check.sh >> /home/vsepulve/gdrift_services/logs/check_services.log
# Notar que en ese comando, "*/5" representa "cada 5 minutos" en la practica
# Idealmente puede agregarse con "crontab -e" (nivel usuario), pero tambien puede agregarse directamente a "/etc/crontab" (de sistema)

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
	echo "$ts: begin checking..."
	check_process "gdrift_service_${1}"
	CHECK_RET=$?
	if [ $CHECK_RET -eq 0 ];
	then
		echo "$ts: Service not running, restarting..."
		killall screen_${1}
		sleep 1
		# Notar que estoy agregando $2 a todos los comandos
		# Como solo entrego ese parametro para controller, los demas usan null y funciona
		screen -dmS screen_${1} /home/gdrift/services/build/bin/gdrift_service_${1} /home/gdrift/services/hosts.json ${2}
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
echo "-----     -----"
check_restart "controller" "0"
echo "-----     -----"
check_restart "scheduler" 
echo "-----     -----"

if [ $restore -eq 1 ];
then
	# send restore command
	echo "RESTORE"
	curl -H "Content-Type: application/json" -X POST http://localhost:1987/scheduler --data-binary "@/home/gdrift/services/test/restore.json"
	echo "-----     -----"
fi

#sleep 3
#done





