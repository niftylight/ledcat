#!/bin/sh

# print video-data as RAW 24-bit RGB values to stdout

# adjust this to match your desired dimensions
WIDTH=16
HEIGHT=16

mencoder -really-quiet -nosound -vf scale=$WIDTH:$HEIGHT,format=rgb24 -of rawvideo -ovc raw -o - $@ 2>/dev/null
