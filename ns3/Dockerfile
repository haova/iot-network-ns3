FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# install dependencies
RUN apt update
RUN apt install -y g++ python3 cmake ninja-build git ccache python3-pip
RUN apt install -y gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3  
RUN apt install -y python3-setuptools git
RUN apt install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
RUN apt install -y python3-gi-cairo
RUN apt install -y openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
RUN apt install -y mercurial unzip
RUN apt install -y gdb valgrind 
RUN apt install -y clang-format
RUN apt install -y doxygen graphviz imagemagick
RUN apt install -y texlive texlive-extra-utils texlive-latex-extra texlive-font-utils dvipng latexmk
RUN apt install -y tcpdump
RUN apt install -y sqlite sqlite3 libsqlite3-dev
RUN apt install -y libxml2 libxml2-dev
RUN apt install -y libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake python3-pip
RUN apt install -y libgtk-3-dev
RUN apt install -y vtun lxc uml-utilities
RUN apt install -y libboost-all-dev
RUN apt install -y wget
RUN apt install -y libssl-dev
RUN apt install -y curl libssl-dev libcurl4-openssl-dev

RUN python3 -m pip install --user cppyy
RUN python3 -m pip install --user cxxfilt

# build cpr
WORKDIR /root
RUN wget https://github.com/libcpr/cpr/archive/refs/tags/1.10.5.tar.gz
RUN tar -xvzf 1.10.5.tar.gz

WORKDIR /root/cpr-1.10.5
RUN mkdir build

WORKDIR /root/cpr-1.10.5/build
RUN cmake .. -DCPR_USE_SYSTEM_CURL=ON
RUN cmake --build . --parallel
RUN cmake --install .

# build ns3
WORKDIR /root
RUN wget https://www.nsnam.org/releases/ns-allinone-3.40.tar.bz2
RUN tar -xvjf ns-allinone-3.40.tar.bz2

WORKDIR /root/ns-allinone-3.40/ns-3.40
ADD ./wifi ./src/wifi
RUN ./ns3 configure --enable-examples --enable-tests --enable-python-bindings
RUN ./ns3 build
ADD ./iotnet ./contrib/iotnet
RUN ln -s ../contrib/iotnet/src scratch/iotnet
RUN ./ns3 configure --enable-examples --enable-tests --enable-python-bindings

RUN mkdir output

# build json
WORKDIR /root
RUN wget https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip
RUN unzip include.zip -d json

WORKDIR /root/ns-allinone-3.40/ns-3.40

ENTRYPOINT ["tail", "-f", "/dev/null"]
