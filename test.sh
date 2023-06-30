#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases

if ! ./testSingle.sh precomp.bC; then
	exit 1
fi

for filename in /y/shared/Engineering/cs-drbc/cs445/testBroad/*.bC; do
	printf $filename
	printf ": "
	if [[ $(diff --text <(realbC $filename) <(./bC $filename)) ]]; then
		printf "NO GOOD:\n"
		diff --text <(realbC $filename) <(./bC $filename)
		printf $filename
		printf "\n"
		exit 1
	else
		printf "All good\n"
	fi
done

for filename in /y/shared/Engineering/cs-drbc/cs445/bC_in_3/*.bC; do
	printf $filename
	printf ": "
	if [[ $(diff --text <(realbC $filename) <(./bC $filename)) ]]; then
		printf "NO GOOD:\n"
		diff --text <(realbC $filename) <(./bC $filename)
		printf $filename
		printf "\n"
		exit 1
	else
		printf "All good\n"
	fi
done

for filename in /y/shared/Engineering/cs-drbc/cs445/testsUnit/*.bC; do
	printf $filename
	printf ": "
	if [[ $(diff --text <(realbC $filename) <(./bC $filename)) ]]; then
		printf "NO GOOD:\n"
		diff --text <(realbC $filename) <(./bC $filename)
		printf $filename
		printf "\n"
		exit 1
	else
		printf "All good\n"
	fi
done
