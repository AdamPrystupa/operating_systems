#!/bin/bash
if [ $# -gt 0 ] && [ "${1,,}" = '-a' ]; then
    ls -A1 | wc -l
elif [ $# -eq 0 ]; then
    ls -1 | wc -l
else
    echo "Podano zły argument! Użyj opcjonalnie '-a', aby zliczyć również ukryte pliki."
fi
