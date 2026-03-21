#!/bin/bash

DIR="/mnt/simple"

for ((i=1; i<=1024; i++)); do
    touch "$DIR/plik_$i"
done