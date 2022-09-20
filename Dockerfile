FROM node:16

RUN apt-get update

ARG root="/root"
WORKDIR $root
ARG version="3.29"

#ns-3 dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y vim build-essential autoconf automake libxmu-dev python-pygraphviz cvs mercurial bzr git cmake p7zip-full python-matplotlib python-tk python-dev python-kiwi qt4-dev-tools qt4-qmake qt4-qmake qt4-default gnuplot-x11 tshark wireshark



#boost installation (required for openflow ns-3 integration)
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.67.0/source/boost_1_67_0.tar.gz && \
    tar xzvf boost_1_67_0.tar.gz && \
    cd boost_1_67_0 && ./bootstrap.sh && ./b2 install && cd .. && rm -rf boost_1_67_0.tar.gz boost_1_67_0

#ns-3 download
RUN wget https://www.nsnam.org/release/ns-allinone-$version.tar.bz2 && \
    tar xjf ns-allinone-$version.tar.bz2 && \
    rm ns-allinone-$version.tar.bz2

#ns-3 build system
RUN apt install -y python3-pip && \
    pip3 install waf

#ns-3 installation
RUN cd ns-allinone-$version/bake && \
    ./bake.py configure -e ns-3-allinone && \
    ./bake.py download && \
    ./bake.py build && \
    cd source/ns-3-dev && \
    ./waf --check-config

#clean up
RUN mkdir ns3 && \
    mv /root/ns-allinone-$version/bake/build/* ns3 && \
    rm -rf /root/ns-allinone-$version 

#updating LD_LIBRARY_PATH and PATH
RUN echo "ns3Path=/root/ns3/lib" >> /root/.bashrc && \
    echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ns3Path' >> /root/.bashrc && \
    echo 'export PATH=$PATH:$ns3Path' >> /root/.bashrc

#installing node-gyp
RUN npm i node-gyp -g
