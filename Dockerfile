FROM ubuntu:bionic

RUN apt-get update && \
    apt-get install -y git \
                       build-essential \
                       wget \
                       cmake \
                       m4 \
                       file \
                       libboost-dev \
                       libopenblas-dev \
                       liblapack-dev \
                       libarpack2-dev \
                       libarpack++2-dev \
					   libarmadillo-dev && \
	TEMPDIR=$(mktemp -d) && \
	wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz && \
	tar xf gmp-6.1.2.tar.xz && \
	cd gmp-6.1.2 && \
	./configure && \
	make && \
	make install && \
	cd $TMPDIR && \
    wget https://www.shoup.net/ntl/ntl-11.3.2.tar.gz && \
	tar xf ntl-11.3.2.tar.gz && \
	cd ntl-11.3.2/src && \
	./configure SHARED=on NTL_GMP_LIP=on NTL_THREADS=on NTL_THREAD_BOOST=on && \
	make -j4 && \
	make install && \
	cd $TMPDIR && \
	mkdir -p /usr/local/src && \
	cd /usr/local/src && \
	git clone https://github.com/homenc/HElib.git && \
	cd HElib && \
	git checkout 63ade33 && \
	cd src && \
	make

WORKDIR /source/src
COPY . /source

RUN make keygen \
		 client \
		 server \
		 update \
		 data_enc \
		 findm \
		 client_datacollect \
		 client_datacollect2_fixedinput \
		 client_datacollect2_generatedinput

CMD ['/bin/bash']
