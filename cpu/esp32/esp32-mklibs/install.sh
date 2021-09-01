#!/bin/sh

ESPIDF_DIR=/opt/esp32/esp-idf
EMBEDX_CPUDIR=../

rm -rf $EMBEDX_CPUDIR/lib
rm -rf $EMBEDX_CPUDIR/include
rm -rf $EMBEDX_CPUDIR/build
rm -rf $EMBEDX_CPUDIR/components

mkdir -p $EMBEDX_CPUDIR/lib
mkdir -p $EMBEDX_CPUDIR/include
mkdir -p $EMBEDX_CPUDIR/build
mkdir -p $EMBEDX_CPUDIR/build/bootloader

find ./build -type f -name "*.a" -exec cp {} $EMBEDX_CPUDIR/lib \;
cp -r ./build/include $EMBEDX_CPUDIR
cp ./build/esp32/esp32_out.ld $EMBEDX_CPUDIR/lib
cp -r $ESPIDF_DIR/components $EMBEDX_CPUDIR/
cp ./build/bootloader/bootloader.bin $EMBEDX_CPUDIR/build/bootloader
cp ./build/partitions* $EMBEDX_CPUDIR/build

echo Files and dirs success copied to .././cpu/esp32 directory

