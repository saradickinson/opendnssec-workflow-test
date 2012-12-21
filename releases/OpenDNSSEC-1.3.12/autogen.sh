#!/bin/sh
#
# $Id$

SUBDIRS="auditor plugins/eppclient"
VERSION=version.m4

if [ `dirname $0` = "." ]; then
	for SUBDIR in ${SUBDIRS}; do
		if [ -d $SUBDIR ]; then
			echo Creating ${SUBDIR}/${VERSION} &&
			rm -f ${SUBDIR}/${VERSION} &&
			ln ${VERSION} ${SUBDIR}/${VERSION} 2>/dev/null ||
			ln -s ${VERSION} ${SUBDIR}/${VERSION} 2>/dev/null ||
			cp ${VERSION} ${SUBDIR}/${VERSION}
		fi
	done
elif [ `dirname $0` = ".." ]; then
	if [ -f ../${VERSION} ]; then
		echo Creating ${VERSION} &&
		rm -f ${VERSION} &&
		ln ../${VERSION} ${VERSION} 2>/dev/null ||
		ln -s ../${VERSION} ${VERSION} 2>/dev/null ||
		cp ../${VERSION} ${VERSION}
	fi
fi &&

echo "Running autoreconf" &&
autoreconf --install --force