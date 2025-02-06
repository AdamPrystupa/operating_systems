#!/bin/bash

export LC_ALL=C

day_of_week=$(date +%u)

if [ "$day_of_week" -le 5 ]; then
	echo "Dzisiaj jest dzien roboczy!" >> zad1.txt
else
	echo "Weekendzik!" >> zad1.txt
fi
