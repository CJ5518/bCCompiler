#!/bin/bash

shopt -s expand_aliases
source ~/.bash_aliases

if ! ./testSingle.sh precomp.bC; then
	exit 1
fi

./testDir.sh /y/shared/Engineering/cs-drbc/cs445/testBroad/
./testDir.sh /y/shared/Engineering/cs-drbc/cs445/testsUnit/
