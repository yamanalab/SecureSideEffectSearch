#!/bin/bash

USAGE="Usage: deploy.sh <target-server-ip> <target-path>"

if [ $# != 2 ]; then
    echo ${USAGE}
    exit 1
fi

TARGET_SERVER=$1
TARGET_PATH=$2

if [ -e src ]; then
    scp -C -r src ${TARGET_SERVER}:${TARGET_PATH}/
else
    echo "Execute on the project root."
fi
