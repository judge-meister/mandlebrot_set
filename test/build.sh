#!/bin/bash

CFILE="$1"

if grep -q gcc "$1"
then
	GCC_CMD=$(grep gcc "$1")

	echo $GCC_CMD
	$GCC_CMD
else
	echo "Not 'C' file or no gcc command in a comment."
fi
