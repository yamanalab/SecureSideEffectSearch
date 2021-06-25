#!/usr/bin/env bash
set -eu

HELIB_INSTALL_DIR="/usr/local/src"
DOWNLOAD_DIR="~/qs_downloads"

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

apt-get update && \
apt-get install -y git \
                   build-essential \
                   wget \
                   cmake \
                   m4 \
                   file \
                   libboost-system-dev* \
                   libopenblas-dev \
                   liblapack-dev \
                   libarpack2-dev \
                   libarpack++2-dev \
                   libarmadillo-dev

mkdir ${DOWNLOAD_DIR}

echo "=============="
echo "Install gmp:"
echo "=============="

cd ${DOWNLOAD_DIR}
wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz && \
  tar xf gmp-6.1.2 && \
  cd gmp-6.1.2 && \
  ./configure && \
  make && \
  make install

echo "=============="
echo "Install NTL:"
echo "=============="

cd ${DOWNLOAD_DIR}
wget https://www.shoup.net/ntl/ntl-11.3.2.tar.gz && \
  tar xf ntl-11.3.2.tar.gz && \
  cd ntl-11.3.2/src && \
  ./configure SHARED=on NTL_GMP_LIP=on NTL_THREADS=on NTL_THREAD_BOOST=on && \
  make -j4 && \
  make install

rm -rf "${DOWNLOAD_DIR}"

echo "=============="
echo "Install HElib:"
echo "=============="

mkdir -p ${HELIB_INSTALL_DIR} && \
  cd ${HELIB_INSTALL_DIR} && \
  git clone https://github.com/homenc/HElib.git && \
  cd HElib && \
  git checkout 8c43c402796495081f83baea49143daa8583371d && \
  cd src && \
  make
