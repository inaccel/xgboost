#!/bin/bash

all:
	scl enable devtoolset-6 "make -C xgboost all"

clean:
	scl enable devtoolset-6 "make -C xgboost clean"

patch:
	patch -Ns xgboost/src/gbm/gbtree.cc inaccel/src/gbm/gbtree.cc.patch
	patch -Ns xgboost/src/gbm/gbtree.h inaccel/src/gbm/gbtree.h.patch
	patch -Ns xgboost/Makefile inaccel/Makefile.patch
	cp inaccel/src/tree/updater_fpga.cc xgboost/src/tree/updater_fpga.cc
