#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases

printf $1
printf ": "
if [[ $(diff --text <(realbC -w $1) <(./bC $1)) ]]; then
	printf "NO GOOD:\n"
	diff --text <(realbC -w $1) <(./bC $1)
	printf $1
	printf "\n"
	exit 1
else
	printf "All good\n"
fi