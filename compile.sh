#!/bin/bash

home=`echo ~`

g++ ./tess.cpp -o tess -L$home/BSC/OpenSourceDev/Tesseract/tesseract_build/lib/ -L$home/clibs/leptonica/lib/ -I$home/BSC/OpenSourceDev/Tesseract/tesseract_build/include/ -I$home/clibs/leptonica/include/ -ltesseract -llept
