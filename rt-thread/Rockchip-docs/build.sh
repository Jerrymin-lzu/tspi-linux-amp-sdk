#!/bin/sh

if [ $# != 1 ]; then
    echo "usage: ./build.sh soc_name. like: ./build.sh rk2108"
    exit 1
else
    echo "building documents for $1"
fi

rm -rf site

cd ../bsp/rockchip/$1
if [ ! -f ./documents/Doxyfile ]; then
    echo "not found './documents/Doxyfile' "
    exit 1
fi
doxygen ./documents/Doxyfile
cp -r ./documents/html ../../../Rockchip-docs/docs/api-reference/driver/

cd ../common/hal/
if [ ! -f ./tools/Doxyfile ] || [ ! -f ./tools/doxyfile_$1 ]; then
    echo "not found 'tools/Doxyfile tools/doxyfile_$1' "
    exit 1
fi
cat tools/Doxyfile tools/doxyfile_$1 | doxygen -
cp -r ./html ../../../../Rockchip-docs/docs/api-reference/hal/

cd ../../../../Rockchip-docs
mkdocs build
echo "Please open the ./site/index.html with web browser like Chrome"
