#!/bin/bash
# add fpga_exact updater
patch -N xgboost/src/gbm/gbtree.cc inaccel/patch/gbtree.cc.patch
patch -N xgboost/src/gbm/gbtree.h inaccel/patch/gbtree.h.patch
# add -lcoral-api to LDFLAGS
patch -N xgboost/Makefile inaccel/patch/Makefile.patch
# copy actual updater to src/
cp -R inaccel/inaccel xgboost/src/
# make
cd xgboost && scl enable devtoolset-7 "make -j8" && cd ..