#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases

for filename in $1/*.bC; do
	./testSingle.sh $filename
done