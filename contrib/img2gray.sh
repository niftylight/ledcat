#!/bin/sh
if test -z "$1" || test -z "$2" ; then echo "Usage $0 infile outfile" ; exit ; fi
#convert $1 -separate -swap 0,1 -combine rgb:$2
convert $1 -separate -combine gray:$2
#convert $1 rgb:$2