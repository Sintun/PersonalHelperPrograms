#!/bin/sh

home=`echo ~`
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$home/clibs/leptonica/lib/:$home/BSC/OpenSourceDev/Tesseract/tesseract_build/lib/:/usr/local/lib64
export LD_LIBRARY_PATH
# $home/BSC/OpenSourceDev/Tesseract/tesseract_build/bin/tesseract -l normal/deu "$1" test1 segdemo inter
# SCROLLVIEW_PATH="$home/BSC/OpenSourceDev/Tesseract/tesseract_build/java/"
# export SCROLLVIEW_PATH
exec ./tableExtractionDemo -f "$1" -d "$2" -l $3

# ~/BSC/OpenSourceDev/Tesseract/tesseract_build/java$ java -jar ./ScrollView.jar
# ./tess.sh ./tab1.tif /usr/share/icolux/tessdata/normal/ deu
