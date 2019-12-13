#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: ./initialize.sh path_to_project_root"
    exit 1
fi

ROOTDIR=$1

function not_exists() {
    test ! -e $1
}

if $(not_exists $ROOTDIR/settings/*.bin); then
    ./keygen
fi

if $(not_exists $ROOTDIR/encdata/*.bin); then
    ./data_enc
fi
