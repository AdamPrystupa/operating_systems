#!/bin/bash

trap '' SIGFPE
trap 'echo WINDOWS CHANGE - redraw' SIGWINCH
trap 'zamykanie' SIGUSR1 
trap 'obsluga_kill' SIGKILL	
temp=0

zamykanie() {
	if [ "$temp" -eq 0 ]; then
		echo "Czy na pewno chcesz zamknac?"
		temp=1
		sleep 10
		temp=0
		else
			echo "Zamknieto projektor"
			exit 0
	fi
}

obsluga_kill() {
	echo "Przechwycono SIGKILL"
	#nie da się przechwycic SIGKILL
}


while true; do
	echo "Projektor PID $$"
	sleep 2
done


