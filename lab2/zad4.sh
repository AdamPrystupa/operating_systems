#!/bin/bash

if [ "$#" -ne 4 ]; then
	echo "Poprawne wywolanie: $0 katalog rozszerzenie liczba_dni nazwa_archiwum"
	exit 1
fi

kat="$1"
roz="$2"
dni="$3"
arch="$4"

if [ ! -d "$kat" ]; then
	echo "Blad: Podany katalog nie istnieje!"
	exit 1
fi

pliki=$(find "$kat" -type f -name "*.$roz" -mtime -"$dni" -perm /u=r 2>/dev/null)

if [ -z "$pliki" ]; then
	echo "Brak plikow spelniajacych kryteria"
	exit 0
fi

tar -cvf "$arch" $pliki

echo "Archiwum o nazwie $arch zostalo pomyslnie utworzone z plikow spelniajacych kryteria: rozszerzenie .$roz, katalog $kat, modyfikowane w przeciagu ostatnich $dni dni."
