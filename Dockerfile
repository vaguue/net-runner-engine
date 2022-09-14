FROM node:16

RUN apt-get update

#ns-3 dependencies
RUN apt-get install -y vim build-essential autoconf automake libxmu-dev python-pygraphviz cvs mercurial bzr git cmake p7zip-full python-matplotlib python-tk python-dev python-kiwi qt4-dev-tools qt4-qmake qt4-qmake qt4-default gnuplot-x11
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y tshark wireshark
ARG root="/root"

WORKDIR $root

#boost installation (required for openflow ns-3 integration)
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.67.0/source/boost_1_67_0.tar.gz
RUN tar xzvf boost_1_67_0.tar.gz
RUN cd boost_1_67_0 && ./bootstrap.sh && ./b2 install && cd ..

#boost clean up
RUN rm boost_1_67_0.tar.gz boost_1_67_0

#ns-3 installation
ARG version="3.29"
RUN wget https://www.nsnam.org/release/ns-allinone-$version.tar.bz2

RUN tar xjf ns-allinone-$version.tar.bz2
RUN rm ns-allinone-$version.tar.bz2

RUN apt install -y python3-pip
RUN pip3 install waf
RUN cd ns-allinone-$version/bake && \
    ./bake.py configure -e ns-3-allinone && \
    ./bake.py download && \
    ./bake.py build && \
    cd source/ns-3-dev && \
    ./waf --check-config

#ns3 clean up
RUN mkdir ns3
RUN mv /root/ns-allinone-$version/bake/build/* ns3
RUN rm -rf /root/ns-allinone-$version

#updating LD_LIBRARY_PATH and PATH
RUN echo "ns3Path=/root/ns3/lib" >> /root/.bashrc
RUN echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ns3Path' >> /root/.bashrc
RUN echo 'export PATH=$PATH:$ns3Path' >> /root/.bashrc

#installing node-gyp
RUN npm i node-gyp -g
