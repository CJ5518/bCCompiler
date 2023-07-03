#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases
if [[ $(diff --text <(realbC -w $1) <(./bC $1)) ]]; then
	echo "NO GOOD:"
	diff --text <(realbC -w $1) <(./bC $1)
else
	echo "All good"
fi