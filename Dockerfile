FROM node:16

RUN apt-get update

#E: Unable to locate package python-pygoocanvas
#E: Package 'python-gnome2' has no installation candidate
#E: Unable to locate package python-gnome2-desktop-dev
#E: Unable to locate package python-rsvg
RUN apt-get install -y vim build-essential autoconf automake libxmu-dev python-pygraphviz cvs mercurial bzr git cmake p7zip-full python-matplotlib python-tk python-dev python-kiwi qt4-dev-tools qt4-qmake qt4-qmake qt4-default gnuplot-x11
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y tshark wireshark
#RUN apt-get install build-essential autoconf automake libxmu-dev python-pygoocanvas python-pygraphviz cvs mercurial bzr git cmake p7zip-full python-matplotlib python-tk python-dev python-kiwi python-gnome2 python-gnome2-desktop-dev python-rsvg qt4-dev-tools qt4-qmake qt4-qmake qt4-default gnuplot-x11 wireshark tshark

#RUN cd $root && wget https://www.nsnam.org/release/ns-allinone-$version.tar.bz2 &> /dev/null;
ARG root="/root"

WORKDIR $root

RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.67.0/source/boost_1_67_0.tar.gz
RUN tar xzvf boost_1_67_0.tar.gz
RUN ls
#RUN mkdir boost && cd boost_1_67_0 && ./bootstrap.sh --prefix=$root/boost && ./b2 install && cd ..
RUN mkdir boost && cd boost_1_67_0 && ./bootstrap.sh && ./b2 install && cd ..

ARG version="3.29"
RUN wget https://www.nsnam.org/release/ns-allinone-$version.tar.bz2

RUN tar xjf ns-allinone-$version.tar.bz2

RUN apt install -y python3-pip
RUN pip3 install waf
RUN cd ns-allinone-$version/bake && \
    ./bake.py configure -e ns-3-allinone && \
    ./bake.py download && \
    ./bake.py build && \
    cd source/ns-3-dev && \
    ./waf --check-config

RUN echo 'path="/root/ns-allinone-3.29/bake/build/lib"' >> /root/.bashrc
RUN echo 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$path' >> /root/.bashrc
RUN echo 'export LD_LIBRARY_PATH' >> /root/.bashrc
RUN npm i node-gyp -g
RUN npm i pm2 -g
CMD ["pm2-runtime", "bin/www"]


#RUN cd $root/ns-allinone-$version && python build.py &> ../build.log && cd ..

#RUN ln -s $root/ns-allinone-/ns-$version $root/NS
#RUN hg clone http://code.nsnam.org/openflow
#RUN cd openflow && ./waf configure && ./waf build
#RUN mkdir openflow/build/default && cp openflow/build/libopenflow.a openflow/build/default

#RUN rm NS
#RUN ln -s $root/ns-allinone-$version/ns-$version $root/NS
#RUN cd ns-allinone-$version/ns-$version && ./waf configure --enable-examples  --with-openflow=$root/openflow --boost-libs=$root/boost/lib --boost-includes=$root/boost/include
#RUN cd ns-allinone-$version/ns-$version && ./waf build
