#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export GOBIN=; 
export GOPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )";

#Build every package in the "src" folder.
echo BUILD PACKAGES
for i in `ls $SCRIPT_DIR/src`
do
	echo $i
	go install $i
done
echo

