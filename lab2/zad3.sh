#!/bin/bash

for i in "$@"; do
	if [ -f $i ]; then
		echo "Plik "$i" istnieje"
	elif [ -d $i ]; then
		if [ -n "ls -a $i" ]; then 
			echo ""$i" jest katalogiem"
		else
			echo ""$i" jest pustym katalogiem"
		fi
		else echo "Nie istnieje plik ani katalog "$i""
	fi
done
	
