#!/bin/bash

if [ $# -gt 0 ] && [ $1 = '-a' ]; then	
	ls -a | wc -l
elif [ $# -eq 0 ]; then
	ls  | wc -l
else
	echo "Podano niepoprawny argument"
fi

