#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases

for filename in /y/shared/Engineering/cs-drbc/cs445/testBroad/*.bC; do
	printf $filename
	printf ": "
	if [[ $(diff --text <(realbC -pcw $filename) <(./bC $filename)) ]]; then
		printf "NO GOOD:\n"
		diff --text <(realbC -pcw $filename) <(./bC $filename)
		printf $filename
		printf "\n"
		exit 1
	else
		printf "All good\n"
	fi
done