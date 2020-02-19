# proc="$((`nproc`-"1"))"
proc="6"
#TESSERACT
#################################################################
# can be commented out after a first build:
cd ~/BSC/OpenSourceDev/Tesseract/tesseract_src/
rm -Rf ~/BSC/OpenSourceDev/Tesseract/tesseract_src/build
mkdir ~/BSC/OpenSourceDev/Tesseract/tesseract_src/build
./autogen.sh
cd ~/BSC/OpenSourceDev/Tesseract/tesseract_src/build
################################################################

home=`echo ~`
PKG_CONFIG_PATH="$home/clibs/leptonica/lib/pkgconfig/:${PKG_CONFIG_PATH}" \
../configure \
--disable-openmp --prefix=$home/BSC/OpenSourceDev/Tesseract/tesseract_build/ \
&&  make -j${proc} && make install
# --disable-legacy
