#!/bin/bash

TRAGET_DIR=$1

if [ -z $TRAGET_DIR ]
then
	echo "please specify an install path as first parameter"
	exit -1
fi
git submodule sync
git submodule init
git submodule update

if [ -z $RRL_INC ]; then
  echo "Environment variable RRL_INC not found. Please set RRL_INC to the include directory of READEX Runtime Library (RRL)"
  exit -1
fi

for d in */ ; do
   if ([ "$d" != "extern/" ]) && ([ "$d" != "scorep_plugin_common/" ]) ; then
	echo "entering $d"
	cd "$d"
	mkdir -p build
	cd build
	cmake ../ -DCMAKE_INSTALL_PREFIX=$TRAGET_DIR
	make
	make install
	cd ../../
   fi
done
