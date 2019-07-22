#!/bin/bash
if [ "$1" = "patch-coral" ]; then
	# add updater to parameters
	patch -Ns xgboost/src/gbm/gbtree.cc src/patch/gbtree.cc.patch
	patch -Ns xgboost/src/gbm/gbtree.h src/patch/gbtree.h.patch
	# add -lcoral-api to LDFLAGS
	patch -Ns xgboost/Makefile src/patch/Makefile.coral.patch
	# copy actual updater to src/
	mkdir xgboost/src/inaccel
	cp src/inaccel/updater_fpga_coral.cc xgboost/src/inaccel
	cd xgboost && make clean
elif [ "$1" = "reverse-coral" ]; then
	patch -Rs xgboost/src/gbm/gbtree.cc src/patch/gbtree.cc.patch
	patch -Rs xgboost/src/gbm/gbtree.h src/patch/gbtree.h.patch
	patch -Rs xgboost/Makefile src/patch/Makefile.coral.patch
	rm -rf xgboost/src/inaccel
	cd xgboost && make clean
elif [ "$1" = "reverse-standalone" ]; then
	patch -Rs xgboost/src/gbm/gbtree.cc src/patch/gbtree.cc.patch
	patch -Rs xgboost/src/gbm/gbtree.h src/patch/gbtree.h.patch
	patch -Rfs xgboost/Makefile src/patch/Makefile.patch
	rm -rf xgboost/src/inaccel
	cd xgboost && make clean
elif [ "$1" = "patch-standalone" ]; then
	# add updater to parameters
	patch -Ns xgboost/src/gbm/gbtree.cc src/patch/gbtree.cc.patch
	patch -Ns xgboost/src/gbm/gbtree.h src/patch/gbtree.h.patch
	# add -lxilinxopencl to LDFLAGS
	patch -Ns xgboost/Makefile src/patch/Makefile.patch
	# copy actual updater to src/
	mkdir xgboost/src/inaccel
	cp src/inaccel/updater_fpga.cc xgboost/src/inaccel
	cp src/inaccel/runtime-api.h xgboost/src/inaccel
	cp src/inaccel/runtime-api.cc xgboost/src/inaccel
	cp src/inaccel/runtime.h xgboost/src/inaccel
	cp src/inaccel/runtime.cc xgboost/src/inaccel
	cp src/inaccel/INcl.h xgboost/src/inaccel
	cp src/inaccel/INcl.cc xgboost/src/inaccel
	cd xgboost && make clean
else
	cd xgboost && scl enable devtoolset-6 "make -j8"
fi