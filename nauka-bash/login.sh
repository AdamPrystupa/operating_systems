#!/bin/bash

case ${1,,} in 
	adam | admin)
		echo "Siema szefie!"
		;;
	help)
		echo "Po prostu podaj swoje imie."
		;;
	*)
		echo "Kim ty jestes? Nie znam cie!"
		
esac

