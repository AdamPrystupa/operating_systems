#!bin/bash

plik="$1"
temat="Wiadomosc testowa"
tresc="Ta wiadomosc zostala wyslana za pomoca skryptu bash!"

while read -r email; do
	if [ -n "$email" ]; then
		echo "$tresc" |  mutt -s "$temat" "$email"
	fi
done < "$plik"

