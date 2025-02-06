#!/bin/bash

syg=$1
echo "SIGFPE   type 1"
echo "SIGWINCH type 2"
echo "SIGUSR1  type 3"

read choice

case $choice in
	1)
		kill -SIGFPE $1
		;;
	2)
		kill -SIGWINCH $1
		;;
	3)
		kill -SIGUSR1 $1
		;;
	*)
		echo "Zly numer!"
		;;
esac

