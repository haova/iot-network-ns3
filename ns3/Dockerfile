FROM ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive

# install dependencies
RUN apt update
RUN apt install -y gnupg2

RUN echo "deb http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu bionic main" >> /etc/apt/sources.list
RUN echo "deb-src http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu bionic main" >> /etc/apt/sources.list
RUN apt-key adv --no-tty --keyserver keyserver.ubuntu.com --recv 60C317803A41BA51845E371A1E9377A2BA9EF27F 

RUN apt update
RUN apt upgrade -y
RUN apt install -y gcc-9 g++-9
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave /usr/bin/g++ g++ /usr/bin/g++-9 --slave /usr/bin/gcov gcov /usr/bin/gcov-9
RUN update-alternatives --config gcc

RUN apt install -y libzmq5 libzmq3-dev
RUN apt purge --auto-remove cmake
RUN apt update
RUN apt install -y software-properties-common lsb-release
RUN apt clean all

RUN apt install -y wget
RUN apt install -y build-essential
RUN apt install -y curl libssl-dev libcurl4-openssl-dev

# cmake
WORKDIR /root
RUN wget https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0.tar.gz
RUN tar -zxvf cmake-3.28.0.tar.gz
WORKDIR /root/cmake-3.28.0
RUN ./bootstrap
RUN make -j$(nproc)
RUN make install

# Install more dependencies
RUN apt-get install -y pkg-config sqlite3 vim
RUN apt-get install -y python3-setuptools git
RUN apt-get install -y qt5-default mercurial
RUN apt-get install -y gdb valgrind
RUN apt-get install -y autoconf cvs bzr unrar
RUN apt-get install -y openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
RUN apt-get install -y gir1.2-goocanvas-2.0 python-gi python-gi-cairo python-pygraphviz python3-gi python3-gi-cairo python3-pygraphviz
RUN apt-get install -y gcc g++ python gcc g++ python python-dev mercurial python-setuptools git qt5-default python-gi python-gi-cairo python-pygraphviz python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython ipython3 openmpi-bin openmpi-common openmpi-doc libopenmpi-dev autoconf cvs bzr unrar gdb valgrind uncrustify doxygen graphviz imagemagick texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng latexmk python-sphinx dia gsl-bin libgslcblas0 libgsl-dev flex bison libfl-dev tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev cmake libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake libgtk2.0-0 libgtk2.0-dev vtun lxc libboost-signals-dev libboost-filesystem-dev g++ python3 python3-dev pkg-config sqlite3 cmake libc6-dev libc6-dev-i386 libclang-6.0-dev llvm-6.0-dev automake python3-pip python-dev python-pygraphviz python-kiwi vim

# Zeromq
WORKDIR /root
RUN wget https://github.com/zeromq/cppzmq/archive/refs/tags/v4.9.0.tar.gz
RUN tar -zxvf v4.9.0.tar.gz
WORKDIR /root/cppzmq-4.9.0/
RUN mkdir build
WORKDIR /root/cppzmq-4.9.0/build
RUN cmake ..
RUN make -j$(nproc)
RUN make install

# python
RUN apt install -y python3.7
RUN update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.7 1
RUN update-alternatives --config python3
RUN update-alternatives --set python3 /usr/bin/python3.7

# minimal requirements for C++:
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y gcc g++ python3 python3-pip cmake
RUN apt-get update
RUN apt-get install -y libzmq5 
RUN apt-get install -y libzmq5-dev
RUN apt-get install -y libzmq3-dev
RUN apt-get install -y libprotobuf-dev
RUN apt-get install -y protobuf-compiler
RUN apt-get install -y qtbase5-dev
RUN apt-get install -y python3-dev libc6-dev g++-multilib
RUN apt-get install -y python3-pip

# protobuf
WORKDIR /root
RUN git clone https://github.com/protocolbuffers/protobuf.git
WORKDIR /root/protobuf
RUN git checkout 3.10.x
RUN git submodule update --init --recursive
RUN ./autogen.sh
RUN ./configure
RUN make -j$(nproc)
RUN make check -j$(nproc)
RUN make install
RUN ldconfig

# ns-3
WORKDIR /root
RUN wget https://www.nsnam.org/releases/ns-allinone-3.30.1.tar.bz2
RUN tar -xvf ns-allinone-3.30.1.tar.bz2
WORKDIR /root/ns-allinone-3.30.1/ns-3.30.1
RUN ./waf configure --enable-examples --enable-tests
RUN ./waf build

# build cpr
WORKDIR /root
RUN wget https://github.com/libcpr/cpr/archive/refs/tags/1.8.4.tar.gz
RUN tar -xvzf 1.8.4.tar.gz

WORKDIR /root/cpr-1.8.4
RUN mkdir build

WORKDIR /root/cpr-1.8.4/build
RUN cmake .. -DCPR_USE_SYSTEM_CURL=ON
RUN cmake --build . --parallel
RUN cmake --install .

# build json
WORKDIR /root
RUN wget https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip
RUN unzip include.zip -d json

# jamming
WORKDIR /root/ns-allinone-3.30.1/ns-3.30.1
ADD jamming ./src/jamming
ADD wifi ./src/wifi

# iotnet
WORKDIR /root/ns-allinone-3.30.1/ns-3.30.1
ADD iotnet ./src/iotnet
RUN ln -s ../src/iotnet/src scratch/iotnet
RUN cp /root/cpr-1.8.4/build/lib/* build/lib/

# build custom modules
RUN ./waf configure
RUN ./waf build 

ADD run.sh .

ENTRYPOINT ["./run.sh"]