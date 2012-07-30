#!/bin/sh

FILENAME="$1"
WIDTH="$2"

if [ -z "${FILENAME}" ] || [ -z "${WIDTH}" ] ; then echo "Usage: $0 <filename> <width-in-pixels>" ; exit 1 ; fi


SUFFIX=${FILENAME##*.}
DIMENSIONS="$(identify $FILENAME | awk '{print $3}')"
IHEIGHT=${DIMENSIONS##*x}
IWIDTH=${DIMENSIONS%%x*}

echo "Loaded image: ${IHEIGHT}px high, ${IWIDTH}px wide"

X=0

for img in $(seq 0 $[IWIDTH-$WIDTH]) ; do
    OUTFILE=$(printf "image%.5d.${SUFFIX}" ${img})
    echo "Writing image ${OUTFILE}"
    convert ${FILENAME}  -crop ${WIDTH}x${IHEIGHT}+${X}+0 ${OUTFILE}
    X=$[X+1]
done
