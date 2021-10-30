#!/bin/bash

TOPDIR=`pwd`
BINDIR=${TOPDIR}/build/demo
CONTEXTFILE=${TOPDIR}/settings/contextsetting.txt
CSVFILE=${TOPDIR}/dmydata/demo.csv

xterm -T "Client" -e "/bin/bash -c 'cd ${BINDIR}/client  && ./client -c ${CONTEXTFILE} 20 m 2815 146; exec /bin/bash -i'"&
xterm -T "Server" -e "/bin/bash -c 'cd ${BINDIR}/server  && ./server -f ${CSVFILE};                   exec /bin/bash -i'"&
