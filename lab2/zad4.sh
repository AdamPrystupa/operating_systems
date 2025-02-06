#!/bin/bash


kat="$1"
roz="$2"
dni="$3"
arch="$4"


pliki=$(find "$kat" -type f -name "*.$roz" -mtime -"$dni" -perm /u=r)


tar -cvf "$arch" $pliki
