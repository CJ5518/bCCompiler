#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases

if ! ./testSingle.sh precomp.bC; then
	exit 1
fi

if ! ./testDir.sh /y/shared/Engineering/cs-drbc/cs445/bC_in_5/su23/; then
	exit 1
fi


