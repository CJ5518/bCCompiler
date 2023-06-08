#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases
if [[ $(diff <(realbC -pc precomp.bC) <(./bC precomp.bC)) ]]; then
	echo "NO GOOD:"
	diff <(realbC -pc precomp.bC) <(./bC precomp.bC)
else
	echo "All good"
fi