#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases

for filename in $1/*.bC; do
	if ! ./testSingle.sh $filename; then
		exit 1
	fi
done