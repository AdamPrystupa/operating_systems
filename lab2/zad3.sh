#!/bin/bash

for arg in "$@"; do
	if [ -f "$arg" ]; then
		echo "plik $arg istnieje"
	elif [ -d "$arg" ]; then
		if [ "$(ls -A "$arg")" ]; then	
			echo "$arg jest katalogiem"
		else
			echo "$arg to pusty katalog" 
		fi
	else
		echo "$arg nie istnieje"
	fi
done
