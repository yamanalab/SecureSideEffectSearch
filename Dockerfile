FROM ubuntu:bionic

ENV LD_LIBRARY_PATH /usr/local/lib

RUN apt-get update &&                       \
    apt-get install -y git                  \
                       build-essential      \
                       wget                 \
                       cmake                \
                       m4                   \
                       file                 \
                       libboost-system-dev* \
                       libopenblas-dev      \
                       liblapack-dev        \
                       libarpack2-dev       \
                       libarpack++2-dev     \
                       libarmadillo-dev

# For Debug
RUN apt-get install -y vim

WORKDIR /tmp
RUN wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.xz &&     \
	tar xf gmp-6.1.2.tar.xz &&                               \
	cd gmp-6.1.2 &&                                          \
	./configure &&                                           \
	make &&                                                  \
	make install

RUN wget https://www.shoup.net/ntl/ntl-11.3.2.tar.gz &&                            \
	tar xf ntl-11.3.2.tar.gz &&                                                \
	cd ntl-11.3.2/src &&                                                       \
	./configure SHARED=on NTL_GMP_LIP=on NTL_THREADS=on NTL_THREAD_BOOST=on && \
	make -j4 &&                                                                \
	make install

RUN mkdir -p /usr/local/src &&                                   \
	cd /usr/local/src &&                                     \
	git clone https://github.com/homenc/HElib.git &&         \
	cd HElib &&                                              \
	git checkout 8c43c402796495081f83baea49143daa8583371d && \
	cd src &&                                                \
	make &&                                                  \
	ln -s fhe.a libhelib.a

WORKDIR /source
COPY . /source

RUN mkdir build && cd build && cmake .. && make -j4

CMD ["/bin/bash"]
