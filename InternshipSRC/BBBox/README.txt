Compilation
-----------

You need libVideoFeatureBridge sources.
If VFB_PATH is the path to sources of libVideoFeatureBridge

mkdir build
cd build
ccmake .. -DCMAKE_MODULE_PATH=${VFB_PATH}/cmake/Modules

