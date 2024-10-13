#!/bin/bash
if [ ${1,,} = adam ]; then
	echo "Witam szefa!"
elif [ ${1,,} = help ]; then
       echo "Po prostu podaj swoje imie!"
else
       echo "Kim ty kurwa jestes?!"
fi 

