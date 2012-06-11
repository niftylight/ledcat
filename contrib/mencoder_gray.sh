#!/bin/sh

# print video-data as RAW 8-bit grayscale values to stdout

# adjust this to match your desired dimensions
WIDTH=256
HEIGHT=64

mencoder -really-quiet -nosound -lavcopts gray -vf scale=$WIDTH:$HEIGHT,format=y8 -of rawvideo -ovc raw -o - $@ 2>/dev/null
