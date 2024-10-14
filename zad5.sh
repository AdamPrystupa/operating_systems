#!bin/bash

if [ "$#" -ne 1]; then
	echo "Poprawne wywolanie: $0 plik_z_adresami"
	exit 1
fi

plik="$1"

if [ ! -f "$plik" ]; then
	echo "Blad: Nie istnieje taki plik!"
	exit 1
fi

temat="Wiadomosc testowa"
tresc="Ta wiadomosc zostala wyslana za pomoca skryptu bash!"

while read -r email; do
	if [ -n "$email" ]; then
		echo "$tresc" | mailx -s "$temat" "$email"
	fi
done < "$plik"
echo "Wiadomosci zostaly wyslane."
