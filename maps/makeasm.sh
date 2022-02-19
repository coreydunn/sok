#!/bin/bash

# Copyright 2022 Corey Dunn
# Creates NASM source file used to
# include all binary files in "direc"
# of format map[0-9].txt
# Usage: ./makeasm.sh MAPDIRECTORY [CHEADERFILE] > file.asm

main()
{
	direc=$1
	cheaderfile=$2

	if [ "$direc" = '' ] || [ ! -d "$direc" ]; then
		echo "error: unkown/nonexistant directory '$direc'"
		exit 1
	fi

	no_maps=$(ls $direc/map*txt|wc -l)
	mapn=imap

	echo "section .data
	"

	for x in $(seq 1 $no_maps); do
		printf "global $mapn$x, $mapn$x""end\n"
		printf "$mapn$x:\nincbin \"$direc/map$x.txt\"\n$mapn$x""end:\n"
	done
	#printf "includemap$i:\n"

	# C HEADER FILE
	if [ $# -gt 1 ]; then
		printf "#define LEVELS $no_maps\n" > $cheaderfile
		for x in $(seq 1 $no_maps); do
			printf "extern char $mapn$x;\n"
			printf "extern char $mapn$x""end;\n"
		done >> $cheaderfile

		printf "char*mapsb[$no_maps];\n" >> $cheaderfile
		printf "char*mapse[$no_maps];\n" >> $cheaderfile

		# Function to programmatically get map data

		printf 'char*get_map(size_t n)'"\n{\n" >> $cheaderfile
		printf '' >> $cheaderfile
		for x in $(seq 1 $no_maps); do
			printf "\tmapsb[$((x-1))]=&$mapn$x;\n" >> $cheaderfile
			printf "\tmapse[$((x-1))]=&$mapn$x""end;\n" >> $cheaderfile
		done >> $cheaderfile
		printf "}\n" >> $cheaderfile
	fi
}

main $*
